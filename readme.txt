given: a directory structure of source with makefiles

1) modify the makefile(s), 
You'll need to set your compiler (CC in the Makefile) to one capable of generating LLVM, e.g `clang`. It it not required to switch to this compiler for your project, but only for this step of identifying SB

typically add to CFLAGS: -S -emit-llvm
with a condition ifeq ($(ll),1)
then anywhere it would make a .o it will build a .ll file instead
also, a second pass with a CFLAGS -g with the conditional
ifeq ($(llg),1)
to get a .llg file

2) run the d2 script, passing the top of the tree it calls make twice, first with ll=1 then a second time w/ llg=1
it generates two files that contain the list of .ll and .llg files
The .ll files are the Basic Block nodes each containting the code for each node
the BB's are named with numeric labels, and each line of the node is unique register name

9:
  %10 = sext i32 %3 to i64
  %11 = getelementptr inbounds i8*, i8** %2, i64 %10
  %12 = sext i32 %1 to i64
  %13 = getelementptr inbounds i8*, i8** %0, i64 %12
  br label %14

The .llg files are similar except that they have debugging information that points each BB to a set of lines in the original source code.

d2 calls the script llg2cdb on each .llg file to create a .csv file used as a mapping from BB to the C source line at the end

3) for each .ll file, we call an llvm utility (opt) to calculate the number (nesting) of each BB in loops
opt -analyze --loops

4) each .ll file is parsed into a .dot file via a script called ll2dot
this custom parser retains information from the .ll file, namely, the function name and label number
additionally, it keeps track of any functions a BB calls
and finally what other nodes (BB) the curent BB can branch to
this forms the directed graph or .dot file which can be directly viewed with the dot utility
Finally, for each function a fake entry and exit node are created to serve as "handles" when the function is called from another function else where in the program

5.1) we call the sb program with the -c option on each .dot file
this generates a list of constraints (list of functions) called by each BB
a constraint is a function that must be realised by an accelerator if the BB that calls it
is chosen to be accelerated.

The user is then presented with each file of constraints, and they should remove any function calls that they want to allow an accelerator to implement. (eg a pure math function like `sqrt` is fine, but an io function like `printf` should remain in the constraint file) These constraint files are cached (and available to future "what if" mods)

5.2) (as a second pass) `sb` is called again on each .dot file, but this time with a -f and the BB's contraint file from the first step. The output of this file is a single file that contains every possible SB

5.3) The single output file is split into one .dot file per SB

6) create a BB database (DB) DB here is a flat file, delimited (usually colon) list of fields
ex:

./lame/lame3.70/quantize.ll:amp_scalefac_bands:58:L_amp_scalefac_bands_270:mul 4:fmul 4:udiv 0:sdiv 0:fdiv 0:urem 0:srem 0:frem 0:
./lame/lame3.70/vbrquantize.ll:VBR_iteration_loop_new:26:L_VBR_iteration_loop_new_116:mul 0:fmul 3:udiv 0:sdiv 0:fdiv 0:urem 0:srem 0:frem 0:llvm_fabs_v2f64,llvm_sqrt_v2f64,llvm_sqrt_v2f64,llvm_fabs_v2f64,llvm_sqrt_v2f64,llvm_sqrt_v2f64,llvm_fabs_v2f64,llvm_sqrt_v2f64,llvm_sqrt_v2f64

7) create a loop DB
ex:

./jpeg/jpeg-6a/jccoefct.ll:compress_first_pass:L_compress_first_pass_186:3


8) create a BB SB DB
ex:

./jpeg/jpeg-6a/jcapimin.ll:L_jpeg_finish_compress_10:0005:0007:0008

9) rank order BB (DB)
ex:

./jpeg/jpeg-6a/jdcoefct.ll:L_decompress_onepass_108:5:0:0027,0028,0034
./jpeg/jpeg-6a/jdcoefct.ll:L_consume_data_98:5:0:0005,0007
./jpeg/jpeg-6a/jdcoefct.ll:L_consume_data_123:5:0:0005

10) separate each BB in the .ll file into individual files

11) create a normalized version of the file. in this step we renumber the registers of the the BB to always start at %1 ... this allows us to compare one BB to another to find similar BBs

12) rank order normalized files (DB)
ex:

./lame/lame3.70/newmdct.bb/L_mdct_sub48_408.norm
./lame/lame3.70/newmdct.bb/L_mdct_sub48_520.norm
./lame/lame3.70/psymodel.bb/L_L3psycho_anal_1857.norm
./lame/lame3.70/newmdct.bb/L_mdct_sub48_488.norm

13) identify SB w/ high ranked BBs (DB)
ex:

./jpeg/jpeg-6a/jdcoefct.db.d/g0061.dot
./jpeg/jpeg-6a/jdcoefct.db.d/g0062.dot
./jpeg/jpeg-6a/jdcoefct.db.d/g0058.dot
./jpeg/jpeg-6a/jdcoefct.db.d/g0059.dot

These SB's can then be realized in Vivado HLS to be further evaluated as a DSA
