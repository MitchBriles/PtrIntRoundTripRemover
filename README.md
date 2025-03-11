# Pointer-Integer Round Trip Remover

is an out-of-tree LLVM pass that collapses instances of pointer-integer round-trips into a single [GEP](https://llvm.org/docs/GetElementPtr.html) (get element pointer)
instruction.

> [!WARNING]  
> This optimization cannot always be done. Certain strict semantics such as [pointer provenance](https://llvm.org/devmtg/2021-11/slides/2021-ptr_provenanceAndLlvmNoaliasTheTaleOfFullRestrict.pdf) make this kind of thing illegal. However, we can get this translation to [verify with physical pointers](https://alive2.llvm.org/ce/z/LQVRnX).

### Building & Running

To build the pass:

```bash
export $LLVM_DIR=<path/to/llvm/19>

cmake .
make
```
To run:
```bash
$LLVM_DIR/bin/opt -load-pass-plugin libPtrIntRoundTripRemover.so --passes="ptr-int-round-trip-remover" -disable-output inputs/simple.ll
```
