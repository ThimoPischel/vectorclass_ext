# vectorclass_ext
Extension header for Agner Fog's SIMD vectorclass

## Overall
The Idea behind this extension is, to make the usage of SIMD easier. This solution will not bring the best possible performance in all situation, but that is not the idea. It should be fast to execute and simple to use. It is pretty fast and espacialy if you have multiple calculation at once, it destroys the old fashion way of for-loop and calculation.

## Why is it based on Agner Fog's vectorclass
There are multiple reasons. The most important one is, it is FAST. From all SIMD Libs I benchmarket, his was the fastest in most of the cases. But performance was not all. The user has to include the vectorclass lib aswell, so it was a perfect match, that it is so simple to include into the project and the documentation of his lib is just amazing. So even if the extension, does not cover some parts you need or you need some more performance, you can easily fall back to his vectorclass.


## Benchmarks
Feel free to run the benchmarks yourself. They are writen in main.cpp. Each system reacts a little bit different and i give no promise for the same performance increase on your system. ALWAYS benchmark on your own (target)-system!

(Ryzen 7 3700U)
|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|          120,445.04 |            8,302.54 |    1.1% |    1,200,385.05 |      462,840.94 |  2.594 |     200,081.04 |    0.0% |      1.49 | `Normal`
|          131,004.65 |            7,633.32 |    0.3% |    1,200,002.07 |      502,752.40 |  2.387 |     100,000.06 |    0.0% |      1.57 | `Normal IN`
|           39,294.89 |           25,448.60 |    1.1% |      519,212.03 |      149,094.56 |  3.482 |     106,346.02 |    0.0% |      0.51 | `Vector Override`
|           15,225.54 |           65,679.10 |    4.2% |      118,817.02 |       55,505.39 |  2.141 |       6,260.01 |    0.0% |      0.18 | `Vector IN`
|           15,277.68 |           65,454.98 |    2.3% |      118,758.02 |       55,619.58 |  2.135 |       6,250.01 |    0.0% |      0.18 | `Compute`
|          788,322.34 |            1,268.52 |    0.4% |    5,500,007.35 |    3,028,585.08 |  1.816 |     100,000.34 |    0.0% |      9.42 | `Complex Normal IN`
|          239,651.40 |            4,172.73 |    0.6% |    3,090,236.11 |      921,343.15 |  3.354 |     638,088.10 |    0.0% |      2.90 | `Complex Vector Override`
|           75,072.08 |           13,320.53 |    0.3% |      581,552.03 |      289,480.19 |  2.009 |      31,297.02 |    0.0% |      0.91 | `Complex Vector IN`
|           42,644.81 |           23,449.51 |    0.2% |      393,808.03 |      164,721.10 |  2.391 |       6,255.02 |    0.0% |      0.57 | `Complex compute`
|           24,306.29 |           41,141.61 |    2.3% |      343,802.02 |       89,993.29 |  3.820 |       6,255.01 |    0.0% |      0.33 | `Complex compute only store e,f`