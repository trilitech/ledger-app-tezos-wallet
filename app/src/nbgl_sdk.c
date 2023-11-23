/* Patch - Ledger Secure SDK

   Copyright 2023 TriliTech <contact@trili.tech>

   With code excerpts from:
   - Ledger Secure SDK <nbgl_use_case.c>, Copyright 2023 Ledger

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#ifdef HAVE_NBGL
#include "nbgl_sdk.h"

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include <stdio.h>
#include "nbgl_debug.h"
#include "nbgl_use_case.h"
#include "glyphs.h"
#include "os_pic.h"
#include "os_helpers.h"
/**********************
 *      TYPEDEFS
 **********************/
enum {
    BACK_TOKEN = 0,
    NEXT_TOKEN,
    QUIT_TOKEN,
    NAV_TOKEN,
    SKIP_TOKEN,
    CONTINUE_TOKEN,
    BUTTON_TOKEN,
    ACTION_BUTTON_TOKEN,
    CHOICE_TOKEN,
    DETAILS_BUTTON_TOKEN,
    CONFIRM_TOKEN,
    REJECT_TOKEN,
    ADDR_BACK_TOKEN,
    ADDR_NEXT_TOKEN
};

/**********************
 *  STATIC VARIABLES
 **********************/
// multi-purposes callbacks
static nbgl_navCallback_t         onNav;
static nbgl_layoutTouchCallback_t onControls;
static nbgl_choiceCallback_t      onChoice;

// contexts for background and modal pages
static nbgl_page_t *pageContext;

// context for settings pages
static nbgl_pageNavigationInfo_t navInfo;
static bool                      forwardNavOnly;

/**********************
 *  STATIC FUNCTIONS
 **********************/
static void displayReviewPage(uint8_t page, bool forceFullRefresh);
static void pageCallback(int token, uint8_t index);

void
nbgl_useCaseForwardOnlyReviewNoSkip(const char                *rejectText,
                                    nbgl_layoutTouchCallback_t buttonCallback,
                                    nbgl_navCallback_t         navCallback,
                                    nbgl_choiceCallback_t      choiceCallback)
{
    // memorize context
    onChoice       = choiceCallback;
    onNav          = navCallback;
    onControls     = buttonCallback;
    forwardNavOnly = true;

    // fill navigation structure
    navInfo.nbPages                  = 0;
    navInfo.navType                  = NAV_WITH_TAP;
    navInfo.quitToken                = REJECT_TOKEN;
    navInfo.navWithTap.nextPageToken = NEXT_TOKEN;
    navInfo.navWithTap.quitText      = rejectText;
    navInfo.navWithTap.backToken     = BACK_TOKEN;
    navInfo.navWithTap.skipText      = NULL;
    navInfo.navWithTap.skipToken     = SKIP_TOKEN;
    navInfo.progressIndicator        = true;
    navInfo.tuneId                   = TUNE_TAP_CASUAL;

    displayReviewPage(0, false);
}

// function used to display the current page in review
static void
displayReviewPage(uint8_t page, bool forceFullRefresh)
{
    nbgl_pageContent_t content = {0};

    // ensure the page is valid
    if ((navInfo.nbPages != 0) && (page >= (navInfo.nbPages))) {
        return;
    }
    navInfo.activePage = page;
    if ((onNav == NULL) || (onNav(navInfo.activePage, &content) == false)) {
        return;
    }

    // override some fields
    content.title            = NULL;
    content.isTouchableTitle = false;
    content.tuneId           = TUNE_TAP_CASUAL;

    if (!forwardNavOnly) {
        navInfo.navWithTap.backButton
            = (navInfo.activePage == 0) ? false : true;
    }

    if (content.type == INFO_LONG_PRESS) {  // last page
        navInfo.navWithTap.nextPageText      = NULL;
        content.infoLongPress.longPressToken = CONFIRM_TOKEN;
        if (forwardNavOnly) {
            // remove the "Skip" button in Footer
            navInfo.navWithTap.skipText = NULL;
        }
    } else {
        navInfo.navWithTap.nextPageText = "Tap to continue";
    }

    // override smallCaseForValue for tag/value types to false
    if (content.type == TAG_VALUE_DETAILS) {
        content.tagValueDetails.tagValueList.smallCaseForValue = false;
        // the maximum displayable number of lines for value is
        // NB_MAX_LINES_IN_REVIEW (without More button)
        content.tagValueDetails.tagValueList.nbMaxLinesForValue
            = NB_MAX_LINES_IN_REVIEW;
    } else if (content.type == TAG_VALUE_LIST) {
        content.tagValueList.smallCaseForValue = false;
    } else if (content.type == TAG_VALUE_CONFIRM) {
        content.tagValueConfirm.tagValueList.smallCaseForValue = false;
        // no next because confirmation is always the last page
        navInfo.navWithTap.nextPageText = NULL;
        // use confirm token for black button
        content.tagValueConfirm.confirmationToken = CONFIRM_TOKEN;
    }

    pageContext
        = nbgl_pageDrawGenericContent(&pageCallback, &navInfo, &content);

    if (forceFullRefresh) {
        nbgl_refreshSpecial(FULL_COLOR_CLEAN_REFRESH);
    } else {
        nbgl_refreshSpecial(FULL_COLOR_PARTIAL_REFRESH);
    }
}

/* Copied from nbgl_use_case.c
 * - most branches remove except for those used by `ForwardOnlyReview`
 */
static void
pageCallback(int token, uint8_t index)
{
    if (token == CHOICE_TOKEN) {
        if (onChoice != NULL) {
            onChoice((index == 0) ? true : false);
        }
    } else if (token == CONFIRM_TOKEN) {
        if (onChoice != NULL) {
            onChoice(true);
        }
    } else if (token == REJECT_TOKEN) {
        if (onChoice != NULL) {
            onChoice(false);
        }
    } else if (token == NEXT_TOKEN) {
        if (onNav != NULL) {
            displayReviewPage(navInfo.activePage + 1, false);
        }
    } else {  // probably a control provided by caller
        if (onControls != NULL) {
            onControls(token, index);
        }
    }
}
#endif  // HAVE_NBGL
