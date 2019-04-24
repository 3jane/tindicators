class Test {
    public static int Main(string[] argv) {
        unsafe {
            void *stream;
            double[] options = new double[]{4};
            TulipIndicators.sma_stream_new(options, &stream);
            var input1 = new double[]{5,6,7,8,7,6,5,4};
            fixed (double *input1_ptr = input1) {
                var inputs = new double*[]{input1_ptr};
                var size = input1.Length;
                var output1 = new double[size];
                fixed (double *output1_ptr = output1) {
                    var outputs = new double*[]{output1_ptr};
                    TulipIndicators.stream_run(stream, size, inputs, outputs);
                    TulipIndicators.sma(size, inputs, options, outputs);
                    TulipIndicators.stream_free(stream);
                }
            }
        }
        return 0;
    }
}
