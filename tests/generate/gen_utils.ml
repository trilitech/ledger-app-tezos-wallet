open Tezos_micheline

let micheline_too_large_or_too_deep expr =
  try
    let rec traverse d expr =
      if d > 40 then raise Exit
      else
        match expr with
        | Micheline.Int (_, n) -> if Z.size n * 64 > 256 then raise Exit
        | Micheline.String _ | Micheline.Bytes _ -> ()
        | Micheline.Prim (_, _, n, _) | Micheline.Seq (_, n) ->
            List.iter (traverse (d + 1)) n
    in
    traverse 0 (Micheline.root expr);
    false
  with Exit -> true
