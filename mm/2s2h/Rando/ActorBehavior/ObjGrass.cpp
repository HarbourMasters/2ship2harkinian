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
          { { -3932.49f, 2092.24f }, RC_TERMINA_FIELD_GRASS_01 },
          { { -3953.84f, 2121.64f }, RC_TERMINA_FIELD_GRASS_02 },
          { { -3995.50f, 2118.37f }, RC_TERMINA_FIELD_GRASS_03 },
          { { -4011.51f, 2079.77f }, RC_TERMINA_FIELD_GRASS_04 },
          { { -3972.00f, 2066.00f }, RC_TERMINA_FIELD_GRASS_05 },
          { { -3900.73f, 2122.33f }, RC_TERMINA_FIELD_GRASS_06 },
          { { -3972.00f, 2166.00f }, RC_TERMINA_FIELD_GRASS_07 },
          { { -4043.28f, 2122.33f }, RC_TERMINA_FIELD_GRASS_08 },
          { { -4043.28f, 2049.67f }, RC_TERMINA_FIELD_GRASS_09 },
          { { -4008.33f, 2014.73f }, RC_TERMINA_FIELD_GRASS_10 },
          { { -3959.52f, 2006.99f }, RC_TERMINA_FIELD_GRASS_11 },
          { { -3929.54f, 2043.61f }, RC_TERMINA_FIELD_GRASS_12 },
          { { -3036.49f, 2613.24f }, RC_TERMINA_FIELD_GRASS_13 },
          { { -3057.84f, 2642.64f }, RC_TERMINA_FIELD_GRASS_14 },
          { { -3099.50f, 2639.37f }, RC_TERMINA_FIELD_GRASS_15 },
          { { -3115.51f, 2600.77f }, RC_TERMINA_FIELD_GRASS_16 },
          { { -3076.00f, 2587.00f }, RC_TERMINA_FIELD_GRASS_17 },
          { { -3004.73f, 2643.33f }, RC_TERMINA_FIELD_GRASS_18 },
          { { -3076.00f, 2687.00f }, RC_TERMINA_FIELD_GRASS_19 },
          { { -3147.28f, 2643.33f }, RC_TERMINA_FIELD_GRASS_20 },
          { { -3147.28f, 2570.68f }, RC_TERMINA_FIELD_GRASS_21 },
          { { -3112.33f, 2535.73f }, RC_TERMINA_FIELD_GRASS_22 },
          { { -3063.53f, 2527.99f }, RC_TERMINA_FIELD_GRASS_23 },
          { { -3033.54f, 2564.61f }, RC_TERMINA_FIELD_GRASS_24 },
          { { -2852.49f, 1585.24f }, RC_TERMINA_FIELD_GRASS_25 },
          { { -2873.84f, 1614.64f }, RC_TERMINA_FIELD_GRASS_26 },
          { { -2915.50f, 1611.37f }, RC_TERMINA_FIELD_GRASS_27 },
          { { -2931.51f, 1572.77f }, RC_TERMINA_FIELD_GRASS_28 },
          { { -2892.00f, 1559.00f }, RC_TERMINA_FIELD_GRASS_29 },
          { { -2820.73f, 1615.33f }, RC_TERMINA_FIELD_GRASS_30 },
          { { -2892.00f, 1659.00f }, RC_TERMINA_FIELD_GRASS_31 },
          { { -2963.28f, 1615.33f }, RC_TERMINA_FIELD_GRASS_32 },
          { { -2963.28f, 1542.68f }, RC_TERMINA_FIELD_GRASS_33 },
          { { -2928.33f, 1507.73f }, RC_TERMINA_FIELD_GRASS_34 },
          { { -2879.52f, 1499.99f }, RC_TERMINA_FIELD_GRASS_35 },
          { { -2849.54f, 1536.61f }, RC_TERMINA_FIELD_GRASS_36 },
          { { -2676.49f, -1136.76f }, RC_TERMINA_FIELD_GRASS_37 },
          { { -2697.84f, -1107.36f }, RC_TERMINA_FIELD_GRASS_38 },
          { { -2739.50f, -1110.63f }, RC_TERMINA_FIELD_GRASS_39 },
          { { -2755.51f, -1149.24f }, RC_TERMINA_FIELD_GRASS_40 },
          { { -2716.00f, -1163.00f }, RC_TERMINA_FIELD_GRASS_41 },
          { { -2644.73f, -1106.67f }, RC_TERMINA_FIELD_GRASS_42 },
          { { -2716.00f, -1063.00f }, RC_TERMINA_FIELD_GRASS_43 },
          { { -2787.27f, -1106.67f }, RC_TERMINA_FIELD_GRASS_44 },
          { { -2787.27f, -1179.33f }, RC_TERMINA_FIELD_GRASS_45 },
          { { -2752.33f, -1214.27f }, RC_TERMINA_FIELD_GRASS_46 },
          { { -2703.52f, -1222.02f }, RC_TERMINA_FIELD_GRASS_47 },
          { { -2673.54f, -1185.39f }, RC_TERMINA_FIELD_GRASS_48 },
          { { -2645.49f, 4303.24f }, RC_TERMINA_FIELD_GRASS_49 },
          { { -2666.84f, 4332.64f }, RC_TERMINA_FIELD_GRASS_50 },
          { { -2708.50f, 4329.37f }, RC_TERMINA_FIELD_GRASS_51 },
          { { -2724.51f, 4290.76f }, RC_TERMINA_FIELD_GRASS_52 },
          { { -2685.00f, 4277.00f }, RC_TERMINA_FIELD_GRASS_53 },
          { { -2613.73f, 4333.33f }, RC_TERMINA_FIELD_GRASS_54 },
          { { -2685.00f, 4377.00f }, RC_TERMINA_FIELD_GRASS_55 },
          { { -2756.27f, 4333.33f }, RC_TERMINA_FIELD_GRASS_56 },
          { { -2756.27f, 4260.67f }, RC_TERMINA_FIELD_GRASS_57 },
          { { -2721.33f, 4225.73f }, RC_TERMINA_FIELD_GRASS_58 },
          { { -2672.52f, 4217.98f }, RC_TERMINA_FIELD_GRASS_59 },
          { { -2642.54f, 4254.61f }, RC_TERMINA_FIELD_GRASS_60 },
          { { -1418.49f, -2599.76f }, RC_TERMINA_FIELD_GRASS_61 },
          { { -1439.84f, -2570.36f }, RC_TERMINA_FIELD_GRASS_62 },
          { { -1481.50f, -2573.63f }, RC_TERMINA_FIELD_GRASS_63 },
          { { -1497.51f, -2612.24f }, RC_TERMINA_FIELD_GRASS_64 },
          { { -1458.00f, -2626.00f }, RC_TERMINA_FIELD_GRASS_65 },
          { { -1386.73f, -2569.67f }, RC_TERMINA_FIELD_GRASS_66 },
          { { -1458.00f, -2526.00f }, RC_TERMINA_FIELD_GRASS_67 },
          { { -1529.27f, -2569.67f }, RC_TERMINA_FIELD_GRASS_68 },
          { { -1529.27f, -2642.33f }, RC_TERMINA_FIELD_GRASS_69 },
          { { -1494.33f, -2677.27f }, RC_TERMINA_FIELD_GRASS_70 },
          { { -1445.52f, -2685.02f }, RC_TERMINA_FIELD_GRASS_71 },
          { { -1415.54f, -2648.39f }, RC_TERMINA_FIELD_GRASS_72 },
          { { -1383.49f, 3960.24f }, RC_TERMINA_FIELD_GRASS_73 },
          { { -1404.84f, 3989.64f }, RC_TERMINA_FIELD_GRASS_74 },
          { { -1446.50f, 3986.37f }, RC_TERMINA_FIELD_GRASS_75 },
          { { -1462.51f, 3947.76f }, RC_TERMINA_FIELD_GRASS_76 },
          { { -1423.00f, 3934.00f }, RC_TERMINA_FIELD_GRASS_77 },
          { { -1351.73f, 3990.33f }, RC_TERMINA_FIELD_GRASS_78 },
          { { -1423.00f, 4034.00f }, RC_TERMINA_FIELD_GRASS_79 },
          { { -1494.27f, 3990.33f }, RC_TERMINA_FIELD_GRASS_80 },
          { { -1494.27f, 3917.67f }, RC_TERMINA_FIELD_GRASS_81 },
          { { -1459.33f, 3882.73f }, RC_TERMINA_FIELD_GRASS_82 },
          { { -1410.52f, 3874.98f }, RC_TERMINA_FIELD_GRASS_83 },
          { { -1380.54f, 3911.61f }, RC_TERMINA_FIELD_GRASS_84 },
          { { -271.49f, 3243.24f }, RC_TERMINA_FIELD_GRASS_85 },
          { { -292.84f, 3272.64f }, RC_TERMINA_FIELD_GRASS_86 },
          { { -334.50f, 3269.37f }, RC_TERMINA_FIELD_GRASS_87 },
          { { -350.51f, 3230.76f }, RC_TERMINA_FIELD_GRASS_88 },
          { { -311.00f, 3217.00f }, RC_TERMINA_FIELD_GRASS_89 },
          { { -239.73f, 3273.33f }, RC_TERMINA_FIELD_GRASS_90 },
          { { -311.00f, 3317.00f }, RC_TERMINA_FIELD_GRASS_91 },
          { { -382.27f, 3273.33f }, RC_TERMINA_FIELD_GRASS_92 },
          { { -382.27f, 3200.67f }, RC_TERMINA_FIELD_GRASS_93 },
          { { -347.33f, 3165.73f }, RC_TERMINA_FIELD_GRASS_94 },
          { { -298.52f, 3157.98f }, RC_TERMINA_FIELD_GRASS_95 },
          { { -268.54f, 3194.61f }, RC_TERMINA_FIELD_GRASS_96 },
          { { 953.51f, -2548.76f }, RC_TERMINA_FIELD_GRASS_97 },
          { { 932.16f, -2519.36f }, RC_TERMINA_FIELD_GRASS_98 },
          { { 890.50f, -2522.63f }, RC_TERMINA_FIELD_GRASS_99 },
          { { 874.49f, -2561.24f }, RC_TERMINA_FIELD_GRASS_100 },
          { { 914.00f, -2575.00f }, RC_TERMINA_FIELD_GRASS_101 },
          { { 985.27f, -2518.67f }, RC_TERMINA_FIELD_GRASS_102 },
          { { 914.00f, -2475.00f }, RC_TERMINA_FIELD_GRASS_103 },
          { { 842.73f, -2518.67f }, RC_TERMINA_FIELD_GRASS_104 },
          { { 842.73f, -2591.33f }, RC_TERMINA_FIELD_GRASS_105 },
          { { 877.67f, -2626.27f }, RC_TERMINA_FIELD_GRASS_106 },
          { { 926.48f, -2634.02f }, RC_TERMINA_FIELD_GRASS_107 },
          { { 956.46f, -2597.39f }, RC_TERMINA_FIELD_GRASS_108 },
          { { 1285.51f, 3183.24f }, RC_TERMINA_FIELD_GRASS_109 },
          { { 1264.16f, 3212.64f }, RC_TERMINA_FIELD_GRASS_110 },
          { { 1222.50f, 3209.37f }, RC_TERMINA_FIELD_GRASS_111 },
          { { 1206.49f, 3170.76f }, RC_TERMINA_FIELD_GRASS_112 },
          { { 1246.00f, 3157.00f }, RC_TERMINA_FIELD_GRASS_113 },
          { { 1317.27f, 3213.33f }, RC_TERMINA_FIELD_GRASS_114 },
          { { 1246.00f, 3257.00f }, RC_TERMINA_FIELD_GRASS_115 },
          { { 1174.73f, 3213.33f }, RC_TERMINA_FIELD_GRASS_116 },
          { { 1174.73f, 3140.67f }, RC_TERMINA_FIELD_GRASS_117 },
          { { 1209.67f, 3105.73f }, RC_TERMINA_FIELD_GRASS_118 },
          { { 1258.48f, 3097.98f }, RC_TERMINA_FIELD_GRASS_119 },
          { { 1288.46f, 3134.61f }, RC_TERMINA_FIELD_GRASS_120 },
          { { 1553.51f, 2053.24f }, RC_TERMINA_FIELD_GRASS_121 },
          { { 1532.16f, 2082.64f }, RC_TERMINA_FIELD_GRASS_122 },
          { { 1490.50f, 2079.37f }, RC_TERMINA_FIELD_GRASS_123 },
          { { 1474.49f, 2040.76f }, RC_TERMINA_FIELD_GRASS_124 },
          { { 1514.00f, 2027.00f }, RC_TERMINA_FIELD_GRASS_125 },
          { { 1585.27f, 2083.33f }, RC_TERMINA_FIELD_GRASS_126 },
          { { 1514.00f, 2127.00f }, RC_TERMINA_FIELD_GRASS_127 },
          { { 1442.73f, 2083.33f }, RC_TERMINA_FIELD_GRASS_128 },
          { { 1442.73f, 2010.67f }, RC_TERMINA_FIELD_GRASS_129 },
          { { 1477.67f, 1975.73f }, RC_TERMINA_FIELD_GRASS_130 },
          { { 1526.48f, 1967.98f }, RC_TERMINA_FIELD_GRASS_131 },
          { { 1556.46f, 2004.61f }, RC_TERMINA_FIELD_GRASS_132 },
          { { 1769.51f, 2736.24f }, RC_TERMINA_FIELD_GRASS_133 },
          { { 1748.16f, 2765.64f }, RC_TERMINA_FIELD_GRASS_134 },
          { { 1706.50f, 2762.37f }, RC_TERMINA_FIELD_GRASS_135 },
          { { 1690.49f, 2723.76f }, RC_TERMINA_FIELD_GRASS_136 },
          { { 1730.00f, 2710.00f }, RC_TERMINA_FIELD_GRASS_137 },
          { { 1801.27f, 2766.33f }, RC_TERMINA_FIELD_GRASS_138 },
          { { 1730.00f, 2810.00f }, RC_TERMINA_FIELD_GRASS_139 },
          { { 1658.73f, 2766.33f }, RC_TERMINA_FIELD_GRASS_140 },
          { { 1658.73f, 2693.67f }, RC_TERMINA_FIELD_GRASS_141 },
          { { 1693.67f, 2658.73f }, RC_TERMINA_FIELD_GRASS_142 },
          { { 1742.48f, 2650.98f }, RC_TERMINA_FIELD_GRASS_143 },
          { { 1772.46f, 2687.61f }, RC_TERMINA_FIELD_GRASS_144 },
          { { 2006.51f, -1294.76f }, RC_TERMINA_FIELD_GRASS_145 },
          { { 1985.16f, -1265.36f }, RC_TERMINA_FIELD_GRASS_146 },
          { { 1943.50f, -1268.63f }, RC_TERMINA_FIELD_GRASS_147 },
          { { 1927.49f, -1307.24f }, RC_TERMINA_FIELD_GRASS_148 },
          { { 1967.00f, -1321.00f }, RC_TERMINA_FIELD_GRASS_149 },
          { { 2038.27f, -1264.67f }, RC_TERMINA_FIELD_GRASS_150 },
          { { 1967.00f, -1221.00f }, RC_TERMINA_FIELD_GRASS_151 },
          { { 1895.73f, -1264.67f }, RC_TERMINA_FIELD_GRASS_152 },
          { { 1895.73f, -1337.33f }, RC_TERMINA_FIELD_GRASS_153 },
          { { 1930.67f, -1372.27f }, RC_TERMINA_FIELD_GRASS_154 },
          { { 1979.48f, -1380.02f }, RC_TERMINA_FIELD_GRASS_155 },
          { { 2009.46f, -1343.39f }, RC_TERMINA_FIELD_GRASS_156 },
          { { 2636.51f, 565.24f }, RC_TERMINA_FIELD_GRASS_157 },
          { { 2615.16f, 594.64f }, RC_TERMINA_FIELD_GRASS_158 },
          { { 2573.50f, 591.37f }, RC_TERMINA_FIELD_GRASS_159 },
          { { 2557.49f, 552.76f }, RC_TERMINA_FIELD_GRASS_160 },
          { { 2597.00f, 539.00f }, RC_TERMINA_FIELD_GRASS_161 },
          { { 2668.27f, 595.33f }, RC_TERMINA_FIELD_GRASS_162 },
          { { 2597.00f, 639.00f }, RC_TERMINA_FIELD_GRASS_163 },
          { { 2525.73f, 595.33f }, RC_TERMINA_FIELD_GRASS_164 },
          { { 2525.73f, 522.67f }, RC_TERMINA_FIELD_GRASS_165 },
          { { 2560.67f, 487.73f }, RC_TERMINA_FIELD_GRASS_166 },
          { { 2609.48f, 479.98f }, RC_TERMINA_FIELD_GRASS_167 },
          { { 2639.46f, 516.61f }, RC_TERMINA_FIELD_GRASS_168 },
          { { 2717.51f, 2414.24f }, RC_TERMINA_FIELD_GRASS_169 },
          { { 2696.16f, 2443.64f }, RC_TERMINA_FIELD_GRASS_170 },
          { { 2654.50f, 2440.37f }, RC_TERMINA_FIELD_GRASS_171 },
          { { 2638.49f, 2401.76f }, RC_TERMINA_FIELD_GRASS_172 },
          { { 2678.00f, 2388.00f }, RC_TERMINA_FIELD_GRASS_173 },
          { { 2749.27f, 2444.33f }, RC_TERMINA_FIELD_GRASS_174 },
          { { 2678.00f, 2488.00f }, RC_TERMINA_FIELD_GRASS_175 },
          { { 2606.73f, 2444.33f }, RC_TERMINA_FIELD_GRASS_176 },
          { { 2606.73f, 2371.67f }, RC_TERMINA_FIELD_GRASS_177 },
          { { 2641.67f, 2336.73f }, RC_TERMINA_FIELD_GRASS_178 },
          { { 2690.48f, 2328.98f }, RC_TERMINA_FIELD_GRASS_179 },
          { { 2720.46f, 2365.61f }, RC_TERMINA_FIELD_GRASS_180 },
          { { 2932.51f, 1832.24f }, RC_TERMINA_FIELD_GRASS_181 },
          { { 2911.16f, 1861.64f }, RC_TERMINA_FIELD_GRASS_182 },
          { { 2869.50f, 1858.37f }, RC_TERMINA_FIELD_GRASS_183 },
          { { 2853.49f, 1819.76f }, RC_TERMINA_FIELD_GRASS_184 },
          { { 2893.00f, 1806.00f }, RC_TERMINA_FIELD_GRASS_185 },
          { { 2964.27f, 1862.33f }, RC_TERMINA_FIELD_GRASS_186 },
          { { 2893.00f, 1906.00f }, RC_TERMINA_FIELD_GRASS_187 },
          { { 2821.73f, 1862.33f }, RC_TERMINA_FIELD_GRASS_188 },
          { { 2821.73f, 1789.67f }, RC_TERMINA_FIELD_GRASS_189 },
          { { 2856.67f, 1754.73f }, RC_TERMINA_FIELD_GRASS_190 },
          { { 2905.48f, 1746.98f }, RC_TERMINA_FIELD_GRASS_191 },
          { { 2935.46f, 1783.61f }, RC_TERMINA_FIELD_GRASS_192 },
          { { 3846.51f, 742.24f }, RC_TERMINA_FIELD_GRASS_193 },
          { { 3825.16f, 771.64f }, RC_TERMINA_FIELD_GRASS_194 },
          { { 3783.50f, 768.37f }, RC_TERMINA_FIELD_GRASS_195 },
          { { 3767.49f, 729.76f }, RC_TERMINA_FIELD_GRASS_196 },
          { { 3807.00f, 716.00f }, RC_TERMINA_FIELD_GRASS_197 },
          { { 3878.27f, 772.33f }, RC_TERMINA_FIELD_GRASS_198 },
          { { 3807.00f, 816.00f }, RC_TERMINA_FIELD_GRASS_199 },
          { { 3735.73f, 772.33f }, RC_TERMINA_FIELD_GRASS_200 },
          { { 3735.73f, 699.67f }, RC_TERMINA_FIELD_GRASS_201 },
          { { 3770.67f, 664.73f }, RC_TERMINA_FIELD_GRASS_202 },
          { { 3819.48f, 656.98f }, RC_TERMINA_FIELD_GRASS_203 },
          { { 3849.46f, 693.61f }, RC_TERMINA_FIELD_GRASS_204 },
          { { 4454.51f, 418.24f }, RC_TERMINA_FIELD_GRASS_205 },
          { { 4433.16f, 447.64f }, RC_TERMINA_FIELD_GRASS_206 },
          { { 4391.50f, 444.37f }, RC_TERMINA_FIELD_GRASS_207 },
          { { 4375.49f, 405.76f }, RC_TERMINA_FIELD_GRASS_208 },
          { { 4415.00f, 392.00f }, RC_TERMINA_FIELD_GRASS_209 },
          { { 4486.27f, 448.33f }, RC_TERMINA_FIELD_GRASS_210 },
          { { 4415.00f, 492.00f }, RC_TERMINA_FIELD_GRASS_211 },
          { { 4343.73f, 448.33f }, RC_TERMINA_FIELD_GRASS_212 },
          { { 4343.73f, 375.67f }, RC_TERMINA_FIELD_GRASS_213 },
          { { 4378.67f, 340.73f }, RC_TERMINA_FIELD_GRASS_214 },
          { { 4427.48f, 332.98f }, RC_TERMINA_FIELD_GRASS_215 },
          { { 4457.46f, 369.61f }, RC_TERMINA_FIELD_GRASS_216 },
      } },
    // Grottos
    { SCENE_KAKUSIANA,
      {
          { { 2284.51f, 883.24f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_01 },
          { { 2263.16f, 912.64f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_02 },
          { { 2221.50f, 909.37f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_03 },
          { { 2205.49f, 870.76f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_04 },
          { { 2245.00f, 857.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_05 },
          { { 2316.27f, 913.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_06 },
          { { 2245.00f, 957.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_07 },
          { { 2173.73f, 913.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_08 },
          { { 2173.73f, 840.67f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_09 },
          { { 2208.67f, 805.73f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_10 },
          { { 2257.48f, 797.98f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_11 },
          { { 2287.46f, 834.61f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_12 },
          { { 2323.51f, 1073.24f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_13 },
          { { 2302.16f, 1102.64f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_14 },
          { { 2260.50f, 1099.37f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_15 },
          { { 2244.49f, 1060.76f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_16 },
          { { 2284.00f, 1047.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_17 },
          { { 2355.27f, 1103.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_18 },
          { { 2284.00f, 1147.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_19 },
          { { 2212.73f, 1103.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_20 },
          { { 2212.73f, 1030.67f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_21 },
          { { 2247.67f, 995.73f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_22 },
          { { 2296.48f, 987.98f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_23 },
          { { 2326.46f, 1024.61f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_24 },
          { { 2437.51f, 993.24f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_25 },
          { { 2416.16f, 1022.64f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_26 },
          { { 2374.50f, 1019.37f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_27 },
          { { 2358.49f, 980.76f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_28 },
          { { 2398.00f, 967.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_29 },
          { { 2469.27f, 1023.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_30 },
          { { 2398.00f, 1067.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_31 },
          { { 2326.73f, 1023.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_32 },
          { { 2326.73f, 950.67f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_33 },
          { { 2361.67f, 915.73f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_34 },
          { { 2410.48f, 907.98f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_35 },
          { { 2440.46f, 944.61f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_36 },
          { { 2465.51f, 747.24f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_37 },
          { { 2444.16f, 776.64f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_38 },
          { { 2402.50f, 773.37f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_39 },
          { { 2386.49f, 734.76f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_40 },
          { { 2426.00f, 721.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_41 },
          { { 2497.27f, 777.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_42 },
          { { 2426.00f, 821.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_43 },
          { { 2354.73f, 777.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_44 },
          { { 2354.73f, 704.67f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_45 },
          { { 2389.67f, 669.73f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_46 },
          { { 2438.48f, 661.98f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_47 },
          { { 2468.46f, 698.61f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_48 },
          { { 2575.51f, 842.24f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_49 },
          { { 2554.16f, 871.64f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_50 },
          { { 2512.50f, 868.37f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_51 },
          { { 2496.49f, 829.76f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_52 },
          { { 2536.00f, 816.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_53 },
          { { 2607.27f, 872.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_54 },
          { { 2536.00f, 916.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_55 },
          { { 2464.73f, 872.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_56 },
          { { 2464.73f, 799.67f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_57 },
          { { 2499.67f, 764.73f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_58 },
          { { 2548.48f, 756.98f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_59 },
          { { 2578.46f, 793.61f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_60 },
          { { 2592.51f, 1073.24f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_61 },
          { { 2571.16f, 1102.64f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_62 },
          { { 2529.50f, 1099.37f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_63 },
          { { 2513.49f, 1060.76f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_64 },
          { { 2553.00f, 1047.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_65 },
          { { 2624.27f, 1103.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_66 },
          { { 2553.00f, 1147.00f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_67 },
          { { 2481.73f, 1103.33f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_68 },
          { { 2481.73f, 1030.67f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_69 },
          { { 2516.67f, 995.73f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_70 },
          { { 2565.48f, 987.98f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_71 },
          { { 2595.46f, 1024.61f }, RC_TERMINA_FIELD_COW_GROTTO_GRASS_72 },
      } },
    // Great Bay Coast
    { SCENE_30GYOSON,
      {
          { { 863.00f, 4646.00f }, RC_GREAT_BAY_COAST_GRASS_01 },
          { { 769.00f, 4647.00f }, RC_GREAT_BAY_COAST_GRASS_02 },
          { { 885.00f, 4881.00f }, RC_GREAT_BAY_COAST_GRASS_03 },
          { { 793.00f, 4880.00f }, RC_GREAT_BAY_COAST_GRASS_04 },
          { { 782.00f, 4766.00f }, RC_GREAT_BAY_COAST_GRASS_05 },
      } }
};

float roundUpToTwoDecimalPlaces(float value) {
    return std::ceil(value * 100.0f) / 100.0f;
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
        ActorId actorId = va_arg(args, ActorId);
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
