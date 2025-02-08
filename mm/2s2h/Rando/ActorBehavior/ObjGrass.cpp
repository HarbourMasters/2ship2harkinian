#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

#include "2s2h/CustomItem/CustomItem.h"
#include "2s2h/Rando/Rando.h"
#include "2s2h/ShipInit.hpp"
#include "assets/2s2h_assets.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_Obj_Grass/z_obj_grass.h"
#include "overlays/actors/ovl_En_Kusa/z_en_kusa.h"

void ObjGrass_Draw(Actor* actor, PlayState* play);
}

std::map<SceneId, std::map<std::pair<float, float>, RandoCheckId>> grassMap = {
    // Termina Field //
    { SCENE_00KEIKOKU,
      {
          { { -3932.00f, 2092.00f }, RC_TERMINA_FIELD_GRASS_01 },
          { { -3954.00f, 2122.00f }, RC_TERMINA_FIELD_GRASS_02 },
          { { -3996.00f, 2118.00f }, RC_TERMINA_FIELD_GRASS_03 },
          { { -4012.00f, 2080.00f }, RC_TERMINA_FIELD_GRASS_04 },
          { { -3972.00f, 2066.00f }, RC_TERMINA_FIELD_GRASS_05 },
          { { -3901.00f, 2122.00f }, RC_TERMINA_FIELD_GRASS_06 },
          { { -3972.00f, 2166.00f }, RC_TERMINA_FIELD_GRASS_07 },
          { { -4043.00f, 2122.00f }, RC_TERMINA_FIELD_GRASS_08 },
          { { -4043.00f, 2050.00f }, RC_TERMINA_FIELD_GRASS_09 },
          { { -4008.00f, 2015.00f }, RC_TERMINA_FIELD_GRASS_10 },
          { { -3960.00f, 2007.00f }, RC_TERMINA_FIELD_GRASS_11 },
          { { -3930.00f, 2044.00f }, RC_TERMINA_FIELD_GRASS_12 },
          { { -3036.00f, 2613.00f }, RC_TERMINA_FIELD_GRASS_13 },
          { { -3058.00f, 2643.00f }, RC_TERMINA_FIELD_GRASS_14 },
          { { -3100.00f, 2639.00f }, RC_TERMINA_FIELD_GRASS_15 },
          { { -3116.00f, 2601.00f }, RC_TERMINA_FIELD_GRASS_16 },
          { { -3076.00f, 2587.00f }, RC_TERMINA_FIELD_GRASS_17 },
          { { -3005.00f, 2643.00f }, RC_TERMINA_FIELD_GRASS_18 },
          { { -3076.00f, 2687.00f }, RC_TERMINA_FIELD_GRASS_19 },
          { { -3147.00f, 2643.00f }, RC_TERMINA_FIELD_GRASS_20 },
          { { -3147.00f, 2571.00f }, RC_TERMINA_FIELD_GRASS_21 },
          { { -3112.00f, 2536.00f }, RC_TERMINA_FIELD_GRASS_22 },
          { { -3064.00f, 2528.00f }, RC_TERMINA_FIELD_GRASS_23 },
          { { -3034.00f, 2565.00f }, RC_TERMINA_FIELD_GRASS_24 },
          { { -2852.00f, 1585.00f }, RC_TERMINA_FIELD_GRASS_25 },
          { { -2874.00f, 1615.00f }, RC_TERMINA_FIELD_GRASS_26 },
          { { -2916.00f, 1611.00f }, RC_TERMINA_FIELD_GRASS_27 },
          { { -2932.00f, 1573.00f }, RC_TERMINA_FIELD_GRASS_28 },
          { { -2892.00f, 1559.00f }, RC_TERMINA_FIELD_GRASS_29 },
          { { -2821.00f, 1615.00f }, RC_TERMINA_FIELD_GRASS_30 },
          { { -2892.00f, 1659.00f }, RC_TERMINA_FIELD_GRASS_31 },
          { { -2963.00f, 1615.00f }, RC_TERMINA_FIELD_GRASS_32 },
          { { -2963.00f, 1543.00f }, RC_TERMINA_FIELD_GRASS_33 },
          { { -2928.00f, 1508.00f }, RC_TERMINA_FIELD_GRASS_34 },
          { { -2880.00f, 1500.00f }, RC_TERMINA_FIELD_GRASS_35 },
          { { -2850.00f, 1537.00f }, RC_TERMINA_FIELD_GRASS_36 },
          { { -2676.00f, -1137.00f }, RC_TERMINA_FIELD_GRASS_37 },
          { { -2698.00f, -1107.00f }, RC_TERMINA_FIELD_GRASS_38 },
          { { -2740.00f, -1111.00f }, RC_TERMINA_FIELD_GRASS_39 },
          { { -2756.00f, -1149.00f }, RC_TERMINA_FIELD_GRASS_40 },
          { { -2716.00f, -1163.00f }, RC_TERMINA_FIELD_GRASS_41 },
          { { -2645.00f, -1107.00f }, RC_TERMINA_FIELD_GRASS_42 },
          { { -2716.00f, -1063.00f }, RC_TERMINA_FIELD_GRASS_43 },
          { { -2787.00f, -1107.00f }, RC_TERMINA_FIELD_GRASS_44 },
          { { -2787.00f, -1179.00f }, RC_TERMINA_FIELD_GRASS_45 },
          { { -2752.00f, -1214.00f }, RC_TERMINA_FIELD_GRASS_46 },
          { { -2704.00f, -1222.00f }, RC_TERMINA_FIELD_GRASS_47 },
          { { -2674.00f, -1185.00f }, RC_TERMINA_FIELD_GRASS_48 },
          { { -2645.00f, 4303.00f }, RC_TERMINA_FIELD_GRASS_49 },
          { { -2667.00f, 4333.00f }, RC_TERMINA_FIELD_GRASS_50 },
          { { -2709.00f, 4329.00f }, RC_TERMINA_FIELD_GRASS_51 },
          { { -2725.00f, 4291.00f }, RC_TERMINA_FIELD_GRASS_52 },
          { { -2685.00f, 4277.00f }, RC_TERMINA_FIELD_GRASS_53 },
          { { -2614.00f, 4333.00f }, RC_TERMINA_FIELD_GRASS_54 },
          { { -2685.00f, 4377.00f }, RC_TERMINA_FIELD_GRASS_55 },
          { { -2756.00f, 4333.00f }, RC_TERMINA_FIELD_GRASS_56 },
          { { -2756.00f, 4261.00f }, RC_TERMINA_FIELD_GRASS_57 },
          { { -2721.00f, 4226.00f }, RC_TERMINA_FIELD_GRASS_58 },
          { { -2673.00f, 4218.00f }, RC_TERMINA_FIELD_GRASS_59 },
          { { -2643.00f, 4255.00f }, RC_TERMINA_FIELD_GRASS_60 },
          { { -1418.00f, -2600.00f }, RC_TERMINA_FIELD_GRASS_61 },
          { { -1440.00f, -2570.00f }, RC_TERMINA_FIELD_GRASS_62 },
          { { -1482.00f, -2574.00f }, RC_TERMINA_FIELD_GRASS_63 },
          { { -1498.00f, -2612.00f }, RC_TERMINA_FIELD_GRASS_64 },
          { { -1458.00f, -2626.00f }, RC_TERMINA_FIELD_GRASS_65 },
          { { -1387.00f, -2570.00f }, RC_TERMINA_FIELD_GRASS_66 },
          { { -1458.00f, -2526.00f }, RC_TERMINA_FIELD_GRASS_67 },
          { { -1529.00f, -2570.00f }, RC_TERMINA_FIELD_GRASS_68 },
          { { -1529.00f, -2642.00f }, RC_TERMINA_FIELD_GRASS_69 },
          { { -1494.00f, -2677.00f }, RC_TERMINA_FIELD_GRASS_70 },
          { { -1446.00f, -2685.00f }, RC_TERMINA_FIELD_GRASS_71 },
          { { -1416.00f, -2648.00f }, RC_TERMINA_FIELD_GRASS_72 },
          { { -1383.00f, 3960.00f }, RC_TERMINA_FIELD_GRASS_73 },
          { { -1405.00f, 3990.00f }, RC_TERMINA_FIELD_GRASS_74 },
          { { -1447.00f, 3986.00f }, RC_TERMINA_FIELD_GRASS_75 },
          { { -1463.00f, 3948.00f }, RC_TERMINA_FIELD_GRASS_76 },
          { { -1423.00f, 3934.00f }, RC_TERMINA_FIELD_GRASS_77 },
          { { -1352.00f, 3990.00f }, RC_TERMINA_FIELD_GRASS_78 },
          { { -1423.00f, 4034.00f }, RC_TERMINA_FIELD_GRASS_79 },
          { { -1494.00f, 3990.00f }, RC_TERMINA_FIELD_GRASS_80 },
          { { -1494.00f, 3918.00f }, RC_TERMINA_FIELD_GRASS_81 },
          { { -1459.00f, 3883.00f }, RC_TERMINA_FIELD_GRASS_82 },
          { { -1411.00f, 3875.00f }, RC_TERMINA_FIELD_GRASS_83 },
          { { -1381.00f, 3912.00f }, RC_TERMINA_FIELD_GRASS_84 },
          { { -271.00f, 3243.00f }, RC_TERMINA_FIELD_GRASS_85 },
          { { -293.00f, 3273.00f }, RC_TERMINA_FIELD_GRASS_86 },
          { { -335.00f, 3269.00f }, RC_TERMINA_FIELD_GRASS_87 },
          { { -351.00f, 3231.00f }, RC_TERMINA_FIELD_GRASS_88 },
          { { -311.00f, 3217.00f }, RC_TERMINA_FIELD_GRASS_89 },
          { { -240.00f, 3273.00f }, RC_TERMINA_FIELD_GRASS_90 },
          { { -311.00f, 3317.00f }, RC_TERMINA_FIELD_GRASS_91 },
          { { -382.00f, 3273.00f }, RC_TERMINA_FIELD_GRASS_92 },
          { { -382.00f, 3201.00f }, RC_TERMINA_FIELD_GRASS_93 },
          { { -347.00f, 3166.00f }, RC_TERMINA_FIELD_GRASS_94 },
          { { -299.00f, 3158.00f }, RC_TERMINA_FIELD_GRASS_95 },
          { { -269.00f, 3195.00f }, RC_TERMINA_FIELD_GRASS_96 },
          { { 954.00f, -2549.00f }, RC_TERMINA_FIELD_GRASS_97 },
          { { 932.00f, -2519.00f }, RC_TERMINA_FIELD_GRASS_98 },
          { { 890.00f, -2523.00f }, RC_TERMINA_FIELD_GRASS_99 },
          { { 874.00f, -2561.00f }, RC_TERMINA_FIELD_GRASS_100 },
          { { 914.00f, -2575.00f }, RC_TERMINA_FIELD_GRASS_101 },
          { { 985.00f, -2519.00f }, RC_TERMINA_FIELD_GRASS_102 },
          { { 914.00f, -2475.00f }, RC_TERMINA_FIELD_GRASS_103 },
          { { 843.00f, -2519.00f }, RC_TERMINA_FIELD_GRASS_104 },
          { { 843.00f, -2591.00f }, RC_TERMINA_FIELD_GRASS_105 },
          { { 878.00f, -2626.00f }, RC_TERMINA_FIELD_GRASS_106 },
          { { 926.00f, -2634.00f }, RC_TERMINA_FIELD_GRASS_107 },
          { { 956.00f, -2597.00f }, RC_TERMINA_FIELD_GRASS_108 },
          { { 1286.00f, 3183.00f }, RC_TERMINA_FIELD_GRASS_109 },
          { { 1264.00f, 3213.00f }, RC_TERMINA_FIELD_GRASS_110 },
          { { 1222.00f, 3209.00f }, RC_TERMINA_FIELD_GRASS_111 },
          { { 1206.00f, 3171.00f }, RC_TERMINA_FIELD_GRASS_112 },
          { { 1246.00f, 3157.00f }, RC_TERMINA_FIELD_GRASS_113 },
          { { 1317.00f, 3213.00f }, RC_TERMINA_FIELD_GRASS_114 },
          { { 1246.00f, 3257.00f }, RC_TERMINA_FIELD_GRASS_115 },
          { { 1175.00f, 3213.00f }, RC_TERMINA_FIELD_GRASS_116 },
          { { 1175.00f, 3141.00f }, RC_TERMINA_FIELD_GRASS_117 },
          { { 1210.00f, 3106.00f }, RC_TERMINA_FIELD_GRASS_118 },
          { { 1258.00f, 3098.00f }, RC_TERMINA_FIELD_GRASS_119 },
          { { 1288.00f, 3135.00f }, RC_TERMINA_FIELD_GRASS_120 },
          { { 1554.00f, 2053.00f }, RC_TERMINA_FIELD_GRASS_121 },
          { { 1532.00f, 2083.00f }, RC_TERMINA_FIELD_GRASS_122 },
          { { 1490.00f, 2079.00f }, RC_TERMINA_FIELD_GRASS_123 },
          { { 1474.00f, 2041.00f }, RC_TERMINA_FIELD_GRASS_124 },
          { { 1514.00f, 2027.00f }, RC_TERMINA_FIELD_GRASS_125 },
          { { 1585.00f, 2083.00f }, RC_TERMINA_FIELD_GRASS_126 },
          { { 1514.00f, 2127.00f }, RC_TERMINA_FIELD_GRASS_127 },
          { { 1443.00f, 2083.00f }, RC_TERMINA_FIELD_GRASS_128 },
          { { 1443.00f, 2011.00f }, RC_TERMINA_FIELD_GRASS_129 },
          { { 1478.00f, 1976.00f }, RC_TERMINA_FIELD_GRASS_130 },
          { { 1526.00f, 1968.00f }, RC_TERMINA_FIELD_GRASS_131 },
          { { 1556.00f, 2005.00f }, RC_TERMINA_FIELD_GRASS_132 },
          { { 1770.00f, 2736.00f }, RC_TERMINA_FIELD_GRASS_133 },
          { { 1748.00f, 2766.00f }, RC_TERMINA_FIELD_GRASS_134 },
          { { 1706.00f, 2762.00f }, RC_TERMINA_FIELD_GRASS_135 },
          { { 1690.00f, 2724.00f }, RC_TERMINA_FIELD_GRASS_136 },
          { { 1730.00f, 2710.00f }, RC_TERMINA_FIELD_GRASS_137 },
          { { 1801.00f, 2766.00f }, RC_TERMINA_FIELD_GRASS_138 },
          { { 1730.00f, 2810.00f }, RC_TERMINA_FIELD_GRASS_139 },
          { { 1659.00f, 2766.00f }, RC_TERMINA_FIELD_GRASS_140 },
          { { 1659.00f, 2694.00f }, RC_TERMINA_FIELD_GRASS_141 },
          { { 1694.00f, 2659.00f }, RC_TERMINA_FIELD_GRASS_142 },
          { { 1742.00f, 2651.00f }, RC_TERMINA_FIELD_GRASS_143 },
          { { 1772.00f, 2688.00f }, RC_TERMINA_FIELD_GRASS_144 },
          { { 2007.00f, -1295.00f }, RC_TERMINA_FIELD_GRASS_145 },
          { { 1985.00f, -1265.00f }, RC_TERMINA_FIELD_GRASS_146 },
          { { 1943.00f, -1269.00f }, RC_TERMINA_FIELD_GRASS_147 },
          { { 1927.00f, -1307.00f }, RC_TERMINA_FIELD_GRASS_148 },
          { { 1967.00f, -1321.00f }, RC_TERMINA_FIELD_GRASS_149 },
          { { 2038.00f, -1265.00f }, RC_TERMINA_FIELD_GRASS_150 },
          { { 1967.00f, -1221.00f }, RC_TERMINA_FIELD_GRASS_151 },
          { { 1896.00f, -1265.00f }, RC_TERMINA_FIELD_GRASS_152 },
          { { 1896.00f, -1337.00f }, RC_TERMINA_FIELD_GRASS_153 },
          { { 1931.00f, -1372.00f }, RC_TERMINA_FIELD_GRASS_154 },
          { { 1979.00f, -1380.00f }, RC_TERMINA_FIELD_GRASS_155 },
          { { 2009.00f, -1343.00f }, RC_TERMINA_FIELD_GRASS_156 },
          { { 2637.00f, 565.00f }, RC_TERMINA_FIELD_GRASS_157 },
          { { 2615.00f, 595.00f }, RC_TERMINA_FIELD_GRASS_158 },
          { { 2573.00f, 591.00f }, RC_TERMINA_FIELD_GRASS_159 },
          { { 2557.00f, 553.00f }, RC_TERMINA_FIELD_GRASS_160 },
          { { 2597.00f, 539.00f }, RC_TERMINA_FIELD_GRASS_161 },
          { { 2668.00f, 595.00f }, RC_TERMINA_FIELD_GRASS_162 },
          { { 2597.00f, 639.00f }, RC_TERMINA_FIELD_GRASS_163 },
          { { 2526.00f, 595.00f }, RC_TERMINA_FIELD_GRASS_164 },
          { { 2526.00f, 523.00f }, RC_TERMINA_FIELD_GRASS_165 },
          { { 2561.00f, 488.00f }, RC_TERMINA_FIELD_GRASS_166 },
          { { 2609.00f, 480.00f }, RC_TERMINA_FIELD_GRASS_167 },
          { { 2639.00f, 517.00f }, RC_TERMINA_FIELD_GRASS_168 },
          { { 2718.00f, 2414.00f }, RC_TERMINA_FIELD_GRASS_169 },
          { { 2696.00f, 2444.00f }, RC_TERMINA_FIELD_GRASS_170 },
          { { 2654.00f, 2440.00f }, RC_TERMINA_FIELD_GRASS_171 },
          { { 2638.00f, 2402.00f }, RC_TERMINA_FIELD_GRASS_172 },
          { { 2678.00f, 2388.00f }, RC_TERMINA_FIELD_GRASS_173 },
          { { 2749.00f, 2444.00f }, RC_TERMINA_FIELD_GRASS_174 },
          { { 2678.00f, 2488.00f }, RC_TERMINA_FIELD_GRASS_175 },
          { { 2607.00f, 2444.00f }, RC_TERMINA_FIELD_GRASS_176 },
          { { 2607.00f, 2372.00f }, RC_TERMINA_FIELD_GRASS_177 },
          { { 2642.00f, 2337.00f }, RC_TERMINA_FIELD_GRASS_178 },
          { { 2690.00f, 2329.00f }, RC_TERMINA_FIELD_GRASS_179 },
          { { 2720.00f, 2366.00f }, RC_TERMINA_FIELD_GRASS_180 },
          { { 2933.00f, 1832.00f }, RC_TERMINA_FIELD_GRASS_181 },
          { { 2911.00f, 1862.00f }, RC_TERMINA_FIELD_GRASS_182 },
          { { 2869.00f, 1858.00f }, RC_TERMINA_FIELD_GRASS_183 },
          { { 2853.00f, 1820.00f }, RC_TERMINA_FIELD_GRASS_184 },
          { { 2893.00f, 1806.00f }, RC_TERMINA_FIELD_GRASS_185 },
          { { 2964.00f, 1862.00f }, RC_TERMINA_FIELD_GRASS_186 },
          { { 2893.00f, 1906.00f }, RC_TERMINA_FIELD_GRASS_187 },
          { { 2822.00f, 1862.00f }, RC_TERMINA_FIELD_GRASS_188 },
          { { 2822.00f, 1790.00f }, RC_TERMINA_FIELD_GRASS_189 },
          { { 2857.00f, 1755.00f }, RC_TERMINA_FIELD_GRASS_190 },
          { { 2905.00f, 1747.00f }, RC_TERMINA_FIELD_GRASS_191 },
          { { 2935.00f, 1784.00f }, RC_TERMINA_FIELD_GRASS_192 },
          { { 3847.00f, 742.00f }, RC_TERMINA_FIELD_GRASS_193 },
          { { 3825.00f, 772.00f }, RC_TERMINA_FIELD_GRASS_194 },
          { { 3783.00f, 768.00f }, RC_TERMINA_FIELD_GRASS_195 },
          { { 3767.00f, 730.00f }, RC_TERMINA_FIELD_GRASS_196 },
          { { 3807.00f, 716.00f }, RC_TERMINA_FIELD_GRASS_197 },
          { { 3878.00f, 772.00f }, RC_TERMINA_FIELD_GRASS_198 },
          { { 3807.00f, 816.00f }, RC_TERMINA_FIELD_GRASS_199 },
          { { 3736.00f, 772.00f }, RC_TERMINA_FIELD_GRASS_200 },
          { { 3736.00f, 700.00f }, RC_TERMINA_FIELD_GRASS_201 },
          { { 3771.00f, 665.00f }, RC_TERMINA_FIELD_GRASS_202 },
          { { 3819.00f, 657.00f }, RC_TERMINA_FIELD_GRASS_203 },
          { { 3849.00f, 694.00f }, RC_TERMINA_FIELD_GRASS_204 },
          { { 4455.00f, 418.00f }, RC_TERMINA_FIELD_GRASS_205 },
          { { 4433.00f, 448.00f }, RC_TERMINA_FIELD_GRASS_206 },
          { { 4391.00f, 444.00f }, RC_TERMINA_FIELD_GRASS_207 },
          { { 4375.00f, 406.00f }, RC_TERMINA_FIELD_GRASS_208 },
          { { 4415.00f, 392.00f }, RC_TERMINA_FIELD_GRASS_209 },
          { { 4486.00f, 448.00f }, RC_TERMINA_FIELD_GRASS_210 },
          { { 4415.00f, 492.00f }, RC_TERMINA_FIELD_GRASS_211 },
          { { 4344.00f, 448.00f }, RC_TERMINA_FIELD_GRASS_212 },
          { { 4344.00f, 376.00f }, RC_TERMINA_FIELD_GRASS_213 },
          { { 4379.00f, 341.00f }, RC_TERMINA_FIELD_GRASS_214 },
          { { 4427.00f, 333.00f }, RC_TERMINA_FIELD_GRASS_215 },
          { { 4457.00f, 370.00f }, RC_TERMINA_FIELD_GRASS_216 },
      } },
    // Grottos
    { SCENE_KAKUSIANA,
      {
          { { 2285.00f, 883.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_01 },
          { { 2263.00f, 913.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_02 },
          { { 2221.00f, 909.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_03 },
          { { 2205.00f, 871.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_04 },
          { { 2245.00f, 857.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_05 },
          { { 2316.00f, 913.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_06 },
          { { 2245.00f, 957.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_07 },
          { { 2174.00f, 913.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_08 },
          { { 2174.00f, 841.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_09 },
          { { 2209.00f, 806.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_10 },
          { { 2257.00f, 798.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_11 },
          { { 2287.00f, 835.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_12 },
          { { 2324.00f, 1073.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_13 },
          { { 2302.00f, 1103.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_14 },
          { { 2260.00f, 1099.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_15 },
          { { 2244.00f, 1061.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_16 },
          { { 2284.00f, 1047.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_17 },
          { { 2355.00f, 1103.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_18 },
          { { 2284.00f, 1147.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_19 },
          { { 2213.00f, 1103.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_20 },
          { { 2213.00f, 1031.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_21 },
          { { 2248.00f, 996.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_22 },
          { { 2296.00f, 988.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_23 },
          { { 2326.00f, 1025.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_24 },
          { { 2438.00f, 993.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_25 },
          { { 2416.00f, 1023.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_26 },
          { { 2374.00f, 1019.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_27 },
          { { 2358.00f, 981.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_28 },
          { { 2398.00f, 967.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_29 },
          { { 2469.00f, 1023.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_30 },
          { { 2398.00f, 1067.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_31 },
          { { 2327.00f, 1023.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_32 },
          { { 2327.00f, 951.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_33 },
          { { 2362.00f, 916.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_34 },
          { { 2410.00f, 908.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_35 },
          { { 2440.00f, 945.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_36 },
          { { 2466.00f, 747.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_37 },
          { { 2444.00f, 777.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_38 },
          { { 2402.00f, 773.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_39 },
          { { 2386.00f, 735.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_40 },
          { { 2426.00f, 721.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_41 },
          { { 2497.00f, 777.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_42 },
          { { 2426.00f, 821.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_43 },
          { { 2355.00f, 777.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_44 },
          { { 2355.00f, 705.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_45 },
          { { 2390.00f, 670.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_46 },
          { { 2438.00f, 662.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_47 },
          { { 2468.00f, 699.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_48 },
          { { 2576.00f, 842.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_49 },
          { { 2554.00f, 872.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_50 },
          { { 2512.00f, 868.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_51 },
          { { 2496.00f, 830.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_52 },
          { { 2536.00f, 816.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_53 },
          { { 2607.00f, 872.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_54 },
          { { 2536.00f, 916.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_55 },
          { { 2465.00f, 872.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_56 },
          { { 2465.00f, 800.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_57 },
          { { 2500.00f, 765.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_58 },
          { { 2548.00f, 757.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_59 },
          { { 2578.00f, 794.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_60 },
          { { 2593.00f, 1073.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_61 },
          { { 2571.00f, 1103.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_62 },
          { { 2529.00f, 1099.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_63 },
          { { 2513.00f, 1061.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_64 },
          { { 2553.00f, 1047.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_65 },
          { { 2624.00f, 1103.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_66 },
          { { 2553.00f, 1147.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_67 },
          { { 2482.00f, 1103.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_68 },
          { { 2482.00f, 1031.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_69 },
          { { 2517.00f, 996.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_70 },
          { { 2565.00f, 988.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_71 },
          { { 2595.00f, 1025.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_72 },
          { { 2339.00f, 129.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_01 },
          { { 2345.00f, -174.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_02 },
          { { 2355.00f, -310.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_03 },
          { { 2355.00f, -225.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_04 },
          { { 2391.00f, -313.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_05 },
          { { 2403.00f, -360.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_06 },
          { { 2460.00f, 215.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_07 },
          { { 2484.00f, -213.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_08 },
          { { 2500.00f, 26.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_09 },
          { { 2508.00f, 152.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_10 },
          { { 2520.00f, -463.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_11 },
          { { 2522.00f, -243.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_12 },
          { { 2524.00f, 58.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_13 },
          { { 2534.00f, -190.00f }, RC_GREAT_BAY_COAST_FISHERMAN_GROTTO_GRASS_14 },
          { { -42.00f, -3.00f }, RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_01 },
          { { 87.00f, -26.00f }, RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_02 },
          { { 103.00f, -112.00f }, RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_03 },
          { { 113.00f, 43.00f }, RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_04 },
          { { 137.00f, 93.00f }, RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_05 },
          { { 2339.00f, 129.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_01 },
          { { 2345.00f, -174.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_02 },
          { { 2355.00f, -310.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_03 },
          { { 2355.00f, -225.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_04 },
          { { 2391.00f, -313.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_05 },
          { { 2403.00f, -360.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_06 },
          { { 2460.00f, 215.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_07 },
          { { 2484.00f, -213.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_08 },
          { { 2500.00f, 26.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_09 },
          { { 2508.00f, 152.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_10 },
          { { 2520.00f, -463.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_11 },
          { { 2522.00f, -243.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_12 },
          { { 2524.00f, 58.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_13 },
          { { 2534.00f, -190.00f }, RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_14 },
          { { 2927.00f, 1308.00f }, RC_TERMINA_FIELD_BIO_BABA_GROTTO_GRASS_01 },
          { { 2966.00f, 1409.00f }, RC_TERMINA_FIELD_BIO_BABA_GROTTO_GRASS_02 },
      } },
    // Great Bay Coast
    { SCENE_30GYOSON,
      {
          { { 863.00f, 4646.00f }, RC_GREAT_BAY_COAST_GRASS_01 },
          { { 769.00f, 4647.00f }, RC_GREAT_BAY_COAST_GRASS_02 },
          { { 885.00f, 4881.00f }, RC_GREAT_BAY_COAST_GRASS_03 },
          { { 793.00f, 4880.00f }, RC_GREAT_BAY_COAST_GRASS_04 },
          { { 782.00f, 4766.00f }, RC_GREAT_BAY_COAST_GRASS_05 },
      } },
    // Clock Town
    { SCENE_ALLEY,
      {
          { { -1900.00f, 316.00f }, RC_CLOCK_TOWN_LAUNDRY_POOL_GRASS_01 },
          { { -1921.00f, 228.00f }, RC_CLOCK_TOWN_LAUNDRY_POOL_GRASS_02 },
          { { -1864.00f, 216.00f }, RC_CLOCK_TOWN_LAUNDRY_POOL_GRASS_03 },
      } },
    // Romani Ranch
    { SCENE_F01,
      {
          { { -135.00f, 1834.00f }, RC_ROMANI_RANCH_GRASS_01 },
          { { -157.00f, 1864.00f }, RC_ROMANI_RANCH_GRASS_02 },
          { { -199.00f, 1860.00f }, RC_ROMANI_RANCH_GRASS_03 },
          { { -215.00f, 1822.00f }, RC_ROMANI_RANCH_GRASS_04 },
          { { -175.00f, 1808.00f }, RC_ROMANI_RANCH_GRASS_05 },
          { { -104.00f, 1864.00f }, RC_ROMANI_RANCH_GRASS_06 },
          { { -175.00f, 1908.00f }, RC_ROMANI_RANCH_GRASS_07 },
          { { -246.00f, 1864.00f }, RC_ROMANI_RANCH_GRASS_08 },
          { { -246.00f, 1792.00f }, RC_ROMANI_RANCH_GRASS_09 },
          { { -211.00f, 1757.00f }, RC_ROMANI_RANCH_GRASS_10 },
          { { -163.00f, 1749.00f }, RC_ROMANI_RANCH_GRASS_11 },
          { { -133.00f, 1786.00f }, RC_ROMANI_RANCH_GRASS_12 },
          { { -1529.00f, 1630.00f }, RC_ROMANI_RANCH_GRASS_13 },
          { { -1551.00f, 1660.00f }, RC_ROMANI_RANCH_GRASS_14 },
          { { -1593.00f, 1656.00f }, RC_ROMANI_RANCH_GRASS_15 },
          { { -1609.00f, 1618.00f }, RC_ROMANI_RANCH_GRASS_16 },
          { { -1569.00f, 1604.00f }, RC_ROMANI_RANCH_GRASS_17 },
          { { -1498.00f, 1660.00f }, RC_ROMANI_RANCH_GRASS_18 },
          { { -1569.00f, 1704.00f }, RC_ROMANI_RANCH_GRASS_19 },
          { { -1640.00f, 1660.00f }, RC_ROMANI_RANCH_GRASS_20 },
          { { -1640.00f, 1588.00f }, RC_ROMANI_RANCH_GRASS_21 },
          { { -1605.00f, 1553.00f }, RC_ROMANI_RANCH_GRASS_22 },
          { { -1557.00f, 1545.00f }, RC_ROMANI_RANCH_GRASS_23 },
          { { -1527.00f, 1582.00f }, RC_ROMANI_RANCH_GRASS_24 },
          { { 1874.00f, 1167.00f }, RC_ROMANI_RANCH_GRASS_25 },
          { { 1852.00f, 1197.00f }, RC_ROMANI_RANCH_GRASS_26 },
          { { 1810.00f, 1193.00f }, RC_ROMANI_RANCH_GRASS_27 },
          { { 1794.00f, 1155.00f }, RC_ROMANI_RANCH_GRASS_28 },
          { { 1834.00f, 1141.00f }, RC_ROMANI_RANCH_GRASS_29 },
          { { 1905.00f, 1197.00f }, RC_ROMANI_RANCH_GRASS_30 },
          { { 1834.00f, 1241.00f }, RC_ROMANI_RANCH_GRASS_31 },
          { { 1763.00f, 1197.00f }, RC_ROMANI_RANCH_GRASS_32 },
          { { 1763.00f, 1125.00f }, RC_ROMANI_RANCH_GRASS_33 },
          { { 1798.00f, 1090.00f }, RC_ROMANI_RANCH_GRASS_34 },
          { { 1846.00f, 1082.00f }, RC_ROMANI_RANCH_GRASS_35 },
          { { 1876.00f, 1119.00f }, RC_ROMANI_RANCH_GRASS_36 },
          { { 1203.00f, -1379.00f }, RC_ROMANI_RANCH_GRASS_37 },
          { { 1181.00f, -1349.00f }, RC_ROMANI_RANCH_GRASS_38 },
          { { 1139.00f, -1353.00f }, RC_ROMANI_RANCH_GRASS_39 },
          { { 1123.00f, -1391.00f }, RC_ROMANI_RANCH_GRASS_40 },
          { { 1163.00f, -1405.00f }, RC_ROMANI_RANCH_GRASS_41 },
          { { 1234.00f, -1349.00f }, RC_ROMANI_RANCH_GRASS_42 },
          { { 1163.00f, -1305.00f }, RC_ROMANI_RANCH_GRASS_43 },
          { { 1092.00f, -1349.00f }, RC_ROMANI_RANCH_GRASS_44 },
          { { 1092.00f, -1421.00f }, RC_ROMANI_RANCH_GRASS_45 },
          { { 1127.00f, -1456.00f }, RC_ROMANI_RANCH_GRASS_46 },
          { { 1175.00f, -1464.00f }, RC_ROMANI_RANCH_GRASS_47 },
          { { 1205.00f, -1427.00f }, RC_ROMANI_RANCH_GRASS_48 },
          { { -223.00f, -2271.00f }, RC_ROMANI_RANCH_GRASS_49 },
          { { -143.00f, -2271.00f }, RC_ROMANI_RANCH_GRASS_50 },
          { { -166.00f, -2214.00f }, RC_ROMANI_RANCH_GRASS_51 },
          { { -223.00f, -2191.00f }, RC_ROMANI_RANCH_GRASS_52 },
          { { -280.00f, -2214.00f }, RC_ROMANI_RANCH_GRASS_53 },
          { { -303.00f, -2271.00f }, RC_ROMANI_RANCH_GRASS_54 },
          { { -280.00f, -2328.00f }, RC_ROMANI_RANCH_GRASS_55 },
          { { -223.00f, -2351.00f }, RC_ROMANI_RANCH_GRASS_56 },
          { { -166.00f, -2328.00f }, RC_ROMANI_RANCH_GRASS_57 },
      }
    },
    // Road to Southern Swamp
    { SCENE_24KEMONOMITI,
      {
          { { 2734.00f, 3399.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_01 },
          { { 2757.00f, 3443.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_02 },
          { { 127.00f, 1839.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_03 },
          { { 47.00f, 1839.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_04 },
          { { 70.00f, 1782.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_05 },
          { { 127.00f, 1759.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_06 },
          { { 184.00f, 1782.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_07 },
          { { 207.00f, 1839.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_08 },
          { { 184.00f, 1896.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_09 },
          { { 127.00f, 1919.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_10 },
          { { 70.00f, 1896.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_11 },
          { { 321.00f, 1647.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_12 },
          { { 241.00f, 1647.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_13 },
          { { 264.00f, 1590.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_14 },
          { { 321.00f, 1567.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_15 },
          { { 378.00f, 1590.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_16 },
          { { 401.00f, 1647.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_17 },
          { { 378.00f, 1704.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_18 },
          { { 321.00f, 1727.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_19 },
          { { 264.00f, 1704.00f }, RC_ROAD_TO_SOUTHERN_SWAMP_GRASS_20 },
      } },
};

float roundUpToTwoDecimalPlaces(float value) {
    int rounded = std::round(value);
    float fnum = static_cast<float>(rounded);

    return fnum;
}

RandoCheckId IdentifyGrass(Vec3f pos) {
    RandoCheckId randoCheckId = RC_UNKNOWN;

    auto it = grassMap.find((SceneId)gPlayState->sceneId);
    if (it == grassMap.end()) {
        return randoCheckId;
    }

    auto innerIt = it->second.find({ pos.x, pos.z });
    if (innerIt == it->second.end()) {
        return randoCheckId;
    }
    return innerIt->second;
}

void SpawnGrassDrop(Vec3f pos, RandoCheckId randoCheckId) {
    CustomItem::Spawn(
        pos.x, pos.y, pos.z, 0, CustomItem::KILL_ON_TOUCH | CustomItem::TOSS_ON_SPAWN | CustomItem::ABLE_TO_ZORA_RANG,
        randoCheckId,
        [](Actor* actor, PlayState* play) {
            RandoSaveCheck& randoSaveCheck = RANDO_SAVE_CHECKS[CUSTOM_ITEM_PARAM];
            randoSaveCheck.eligible = true;
        },
        [](Actor* actor, PlayState* play) {
            auto& randoSaveCheck = RANDO_SAVE_CHECKS[CUSTOM_ITEM_PARAM];
            RandoItemId randoItemId = Rando::ConvertItem(randoSaveCheck.randoItemId);
            Matrix_Scale(30.0f, 30.0f, 30.0f, MTXMODE_APPLY);
            Rando::DrawItem(Rando::ConvertItem(randoSaveCheck.randoItemId, (RandoCheckId)CUSTOM_ITEM_PARAM), actor);
        });
}

// void ObjGrass_RandoDraw(Actor* actor, PlayState* play) {
//     if (!CVarGetInteger("gRando.CSMC", 0)) {
//         Gfx_DrawDListOpa(play, (Gfx*)gPotStandardDL);
//         return;
//     }
//
//     RandoItemId randoItemId = Rando::ConvertItem(RANDO_SAVE_CHECKS[OBJGRASS_RC].randoItemId,
//     (RandoCheckId)OBJGRASS_RC); RandoItemType randoItemType = Rando::StaticData::Items[randoItemId].randoItemType;
//
//     switch (randoItemType) {
//         case RITYPE_BOSS_KEY:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotBossKeyDL);
//             break;
//         case RITYPE_HEALTH:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotHeartDL);
//             break;
//         case RITYPE_LESSER:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotMinorDL);
//             break;
//         case RITYPE_MAJOR:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotMajorDL);
//             break;
//         case RITYPE_MASK:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotMaskDL);
//             break;
//         case RITYPE_SKULLTULA_TOKEN:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotTokenDL);
//             break;
//         case RITYPE_SMALL_KEY:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotSmallKeyDL);
//             break;
//         case RITYPE_STRAY_FAIRY:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotFairyDL);
//             break;
//         default:
//             Gfx_DrawDListOpa(play, (Gfx*)gPotStandardDL);
//             break;
//     }
// }

void Rando::ActorBehavior::InitObjGrassBehavior() {
    /*COND_VB_SHOULD(VB_GRASS_DRAW_BE_OVERRIDDEN, IS_RANDO, {
        Actor* actor = va_arg(args, Actor*);
        if (OBJGRASS_RC != RC_UNKNOWN) {
            *should = false;
            actor->draw = ObjGrass_RandoDraw;
        }
    });*/

    COND_VB_SHOULD(VB_GRASS_DROP_COLLECTIBLE, IS_RANDO, {
        auto actorId = static_cast<ActorId>(va_arg(args, int32_t));
        ObjGrassElement* grassElemActor;
        EnKusa* kusaActor;
        Vec3f pos;

        if (actorId == ACTOR_OBJ_GRASS) {
            grassElemActor = va_arg(args, ObjGrassElement*);
            pos = grassElemActor->pos;
        } else {
            kusaActor = va_arg(args, EnKusa*);
            pos = kusaActor->actor.home.pos;
        }

        pos.x = roundUpToTwoDecimalPlaces(pos.x);
        pos.z = roundUpToTwoDecimalPlaces(pos.z);

        RandoCheckId randoCheckId = IdentifyGrass(pos);
        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        SpawnGrassDrop(pos, randoCheckId);
        *should = false;
    });
}
