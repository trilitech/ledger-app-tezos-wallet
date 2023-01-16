open Tezos_client_015_PtLimaPt

let preprocess_pattern_string p =
  let rewrite p =
    let rec try_metas = function
      | [] -> Stdlib.failwith "the pattern is bad"
      | (prefix, n) :: metas ->
          if String.starts_with ~prefix p then
            let l = String.length prefix in
            let def = String.sub p l (String.length p - l) in
            "(sapling_transaction_deprecated " ^ string_of_int n ^ def ^ ")"
          else try_metas metas
    in
    let rec try_caps metas = function
      | [] -> try_metas metas
      | (prefix, n) :: caps ->
          if String.starts_with ~prefix p then
            let l = String.length prefix in
            let name = String.sub p l (String.length p - l) in
            let name = String.trim name in
            "(sapling_transaction_deprecated " ^ string_of_int n ^ " \"" ^ name
            ^ "\")"
          else try_caps metas caps
    in
    try_caps
      [ ("list", 62); ("or", 63) ]
      [ ("any", 0); ("bytes", 1); ("int", 2); ("string", 3); ("address", 4) ]
  in
  let rec fix p =
    let p' =
      let b = Buffer.create 0 in
      Buffer.add_substitute b rewrite p;
      Buffer.contents b
    in
    if p = p' then p else fix p'
  in
  fix p

let read_pattern input =
  let input = preprocess_pattern_string input in
  let parsed =
    Tezos_micheline.Micheline_parser.no_parsing_error
      (Michelson_v1_parser.parse_expression input)
  in
  match parsed with
  | Error _ -> Stdlib.failwith "cannot parse pattern"
  | Ok { expanded; _ } -> expanded
