using System;
using System.Collections.Generic;
using System.Linq;
using QuantConnect.Indicators;
using Rcdb;

namespace Test {
    public class Test {
        public static int Main(string[] argv) {
            {
                var data = new[] {1,2,3,4,5,6,7,8,9,10};
                var sma = new Rcdb.TulipIndicators.Streaming.sma(4);
                for (int i = 0; i < data.Length; i++) {
                    sma.Update(new IndicatorDataPoint(DateTime.Now.AddSeconds(i), data[i]));
                    Console.Write(sma.SMA.Current.Value); Console.Write(" ");
                }
            }

            Console.Write("\n");

            {
                var data = new[] {1,2,3,4,5,6,7,8,9,10};
                var sma = new Rcdb.TulipIndicators.Default.sma(4);
                for (int i = 0; i < data.Length; i++) {
                    sma.Update(new IndicatorDataPoint(DateTime.Now.AddSeconds(i), data[i]));
                    Console.Write(sma.SMA.Current.Value); Console.Write(" ");
                }
            }

            return 0;
        }
    }
}
