(*open Tezos_rpc_http_client_unix

    let get = RPC_client_unix.call_service

  open Lwt.Infix
*)

let () =
  assert (Array.length Sys.argv > 2);
  Lwt_main.run
    (Lwt_process.with_process_none
       (Sys.argv.(2), Array.sub Sys.argv 2 (Array.length Sys.argv - 2))
       (fun _pro -> Lwt.return ()))
