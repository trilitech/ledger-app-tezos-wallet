let preprocess_pattern input =
  let input = Process_pattern.read_pattern input in
  let bin =
    Data_encoding.Binary.to_bytes_exn
      Tezos_protocol_015_PtLimaPt.Protocol.Script_repr.expr_encoding input
  in
  Format.printf "%a" Hex.pp (Hex.of_bytes bin)

let () = preprocess_pattern Sys.argv.(1)
