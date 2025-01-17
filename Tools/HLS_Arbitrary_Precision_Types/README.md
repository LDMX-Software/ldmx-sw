# HLS Arbitrary Precision Types

Originating from [`Xilinx/HLS_arbitrary_Precision_Types`](https://github.com/Xilinx/HLS_arbitrary_Precision_Types)
commit 200a9ae.

Keeping these headers separate from our other Tools headers for two reasons.
1. They originated from somewhere else and have a different (although very permissive) license.
2. They do not pass our strigent compilation requirements so we have CMake pretend
   they are system headers.

The Apache 2 license requires us to publish the changes we've made to the source code we have
copied. This requirement is satisifed by keeping our commits to ldmx-sw public. The other
requirement is keeping the Apache 2 license on the copied source code which we can maintain
by keeping this code separate.

The [original README](OLD_README.MD) is available as well.
