# Pointer-Integer Round Trip Remover

is an out-of-tree LLVM pass that collapses instanses of pointer-integer round-trips into a single GEP (get element pointer)
instruction.

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