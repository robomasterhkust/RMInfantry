#include "function_list.h"
#include "global_variable.h"
#include "customized_function.h"
#include "external_control.h"
#include "chassis_control.h"
#include "gimbal_control.h"

void pause(int ms) {
	u32 ticks = get_ms_ticks();
	while (get_ms_ticks() == ticks);
	while ((get_ms_ticks() - ticks) % ms != 0);
}

void limit_int_range(int16_t* num, int16_t upper_bound, int16_t lower_bound) {
	if (*num > upper_bound)
		*num = upper_bound;
	else if (*num < lower_bound)
		*num = lower_bound;
}

void limit_s32_range(s32* num, s32 upper_bound, s32 lower_bound) {
	if (*num > upper_bound)
		*num = upper_bound;
	else if (*num < lower_bound)
		*num = lower_bound;
}

void limit_float_range(float* num, float upper_bound, float lower_bound)
{
	if (*num > upper_bound)
		*num = upper_bound;
	else if (*num < lower_bound)
		*num = lower_bound;
}

int16_t float_equal(float x, float y, float delta)
{
	if (x - y < delta && y - x < delta)
	{
		return 1;
	}
	else return 0;
}

void PID_init_all(void)
{
	PID_init_chassis();
	PID_init_gimbal();
	PID_init_driver();
}

void PID_init_chassis(void)
{
	for (int i = 0; i < 4; i++) {
		PID_init(&wheels_pos_pid[i], 0.15, 0, 0, 20000, Positional);
		PID_init(&wheels_speed_pid[i], 80, 5, 100, 20000, IntegralResist); //0.00001
		PID_SetIntegral(&wheels_speed_pid[i], 990, -990, 100, 15, 0, 0, 0);
	}
	PID_init(&angle_pid, 4, 0, 40, 990, Positional); //4 is best 660 is some limit
	// PID_init(&buffer_pid, 0.005, 0.0035, 0, 0.7);
	PID_init(&buffer_pid, 0.006, 0.00055, 0, 0.7, IntegralSegment);
	PID_init(&gimbal_reset_pid, 0.02, 0, 0, 50, Positional);
	target_angle = current_angle;
	gimbal_follow = 0;
}

void PID_init_gimbal(void)
{
	PID_init(&gimbal_speed_pid[0], 80, 5, 100, 20000, IntegralResist);
	PID_SetIntegral(&gimbal_speed_pid[0], 660, -660, 100, 15, 0, 0, 0);
	PID_init(&gimbal_speed_pid[1], 80, 5, 100, 20000, IntegralResist);
	PID_SetIntegral(&gimbal_speed_pid[1], 660, -660, 100, 15, 0, 0, 0);
	PID_init(&gimbal_pos_pid[0], 0.18, 0, 0, 20000, Positional);
	PID_init(&gimbal_pos_pid[1], 0.35, 0, 0, 20000, Positional);
	PID_init(&mouse_input_pid, 0.00185, 0, 0, 1, Positional);
}

void PID_init_driver(void)
{
	// PID_init(&gun_driver_speed_pid, 80, 5, 100, 8000, IntegralResist);
	// PID_SetIntegral(&gun_driver_speed_pid, 2000, -2000, 100, 15, 0, 0, 0);
	PID_init(&gun_driver_speed_pid, 80, 0.5, 100, 6000, IntegralSegment);
	PID_init(&gun_driver_pos_pid, 0.10, 0, 0, 2000, Positional);
}

void PID_Reset_all(void)
{
	PID_Reset_chassis();
	PID_Reset_gimbal();
	PID_Reset_driver();
}

void PID_Reset_chassis(void)
{
	for (int i = 0; i < 4; i++) {
		PID_ResetValue(&wheels_pos_pid[i]);
		PID_ResetValue(&wheels_speed_pid[i]); //0.00001
	}
	PID_ResetValue(&angle_pid); //4 is best 660 is some limit
	// PID_init(&buffer_pid, 0.005, 0.0035, 0, 0.7);
	PID_ResetValue(&buffer_pid);
	PID_ResetValue(&gimbal_reset_pid);
	target_angle = current_angle;
	gimbal_follow = 0;
}

void PID_Reset_gimbal(void)
{
	PID_ResetValue(&gimbal_speed_pid[0]);
	PID_ResetValue(&gimbal_speed_pid[1]);
	PID_ResetValue(&gimbal_pos_pid[0]);
	PID_ResetValue(&gimbal_pos_pid[1]);
}

void PID_Reset_driver(void)
{
	PID_ResetValue(&gun_driver_speed_pid);
	PID_ResetValue(&gun_driver_pos_pid);
	GUN_TargetPos = GMxEncoder.ecd_angle;
	gun_driver_input = 0;
}

float sin_table[900] = {0, 0.00174533, 0.00349065, 0.00523596, 0.00698126, 0.00872654, 0.0104718, 0.012217, 0.0139622, 0.0157073, 0.0174524, 0.0191974, 0.0209424, 0.0226873, 0.0244322, 0.0261769, 0.0279216, 0.0296662, 0.0314108, 0.0331552, 0.0348995, 0.0366437, 0.0383878, 0.0401318, 0.0418757, 0.0436194, 0.045363, 0.0471065, 0.0488498, 0.0505929, 0.052336, 0.0540788, 0.0558215, 0.057564, 0.0593064, 0.0610485, 0.0627905, 0.0645323, 0.0662739, 0.0680153, 0.0697565, 0.0714974, 0.0732382, 0.0749787, 0.076719, 0.0784591, 0.0801989, 0.0819385, 0.0836778, 0.0854169, 0.0871557, 0.0888943, 0.0906326, 0.0923706, 0.0941083, 0.0958458, 0.0975829, 0.0993197, 0.101056, 0.102793, 0.104528, 0.106264, 0.107999, 0.109734, 0.111469, 0.113203, 0.114937, 0.116671, 0.118404, 0.120137, 0.121869, 0.123601, 0.125333, 0.127065, 0.128796, 0.130526, 0.132256, 0.133986, 0.135716, 0.137445, 0.139173, 0.140901, 0.142629, 0.144356, 0.146083, 0.147809, 0.149535, 0.151261, 0.152986, 0.15471, 0.156434, 0.158158, 0.159881, 0.161604, 0.163326, 0.165048, 0.166769, 0.168489, 0.170209, 0.171929, 0.173648, 0.175367, 0.177085, 0.178802, 0.180519, 0.182236, 0.183951, 0.185667, 0.187381, 0.189095, 0.190809, 0.192522, 0.194234, 0.195946, 0.197657, 0.199368, 0.201078, 0.202787, 0.204496, 0.206204, 0.207912, 0.209619, 0.211325, 0.21303, 0.214735, 0.21644, 0.218143, 0.219846, 0.221548, 0.22325, 0.224951, 0.226651, 0.228351, 0.23005, 0.231748, 0.233445, 0.235142, 0.236838, 0.238533, 0.240228, 0.241922, 0.243615, 0.245307, 0.246999, 0.24869, 0.25038, 0.252069, 0.253758, 0.255446, 0.257133, 0.258819, 0.260505, 0.262189, 0.263873, 0.265556, 0.267238, 0.26892, 0.2706, 0.27228, 0.273959, 0.275637, 0.277315, 0.278991, 0.280667, 0.282341, 0.284015, 0.285688, 0.287361, 0.289032, 0.290702, 0.292372, 0.29404, 0.295708, 0.297375, 0.299041, 0.300706, 0.30237, 0.304033, 0.305695, 0.307357, 0.309017, 0.310676, 0.312335, 0.313992, 0.315649, 0.317305, 0.318959, 0.320613, 0.322266, 0.323917, 0.325568, 0.327218, 0.328867, 0.330514, 0.332161, 0.333807, 0.335452, 0.337095, 0.338738, 0.34038, 0.34202, 0.34366, 0.345298, 0.346936, 0.348572, 0.350207, 0.351842, 0.353475, 0.355107, 0.356738, 0.358368, 0.359997, 0.361625, 0.363251, 0.364877, 0.366501, 0.368125, 0.369747, 0.371368, 0.372988, 0.374607, 0.376224, 0.377841, 0.379456, 0.38107, 0.382683, 0.384295, 0.385906, 0.387516, 0.389124, 0.390731, 0.392337, 0.393942, 0.395546, 0.397148, 0.398749, 0.400349, 0.401948, 0.403545, 0.405142, 0.406737, 0.40833, 0.409923, 0.411514, 0.413104, 0.414693, 0.416281, 0.417867, 0.419452, 0.421036, 0.422618, 0.424199, 0.425779, 0.427358, 0.428935, 0.430511, 0.432086, 0.433659, 0.435231, 0.436802, 0.438371, 0.439939, 0.441506, 0.443071, 0.444635, 0.446198, 0.447759, 0.449319, 0.450878, 0.452435, 0.45399, 0.455545, 0.457098, 0.45865, 0.4602, 0.461749, 0.463296, 0.464842, 0.466387, 0.46793, 0.469472, 0.471012, 0.472551, 0.474088, 0.475624, 0.477159, 0.478692, 0.480223, 0.481754, 0.483282, 0.48481, 0.486335, 0.48786, 0.489382, 0.490904, 0.492424, 0.493942, 0.495459, 0.496974, 0.498488, 0.5, 0.501511, 0.50302, 0.504528, 0.506034, 0.507538, 0.509041, 0.510543, 0.512043, 0.513541, 0.515038, 0.516533, 0.518027, 0.519519, 0.52101, 0.522499, 0.523986, 0.525472, 0.526956, 0.528438, 0.529919, 0.531399, 0.532876, 0.534352, 0.535827, 0.5373, 0.538771, 0.54024, 0.541708, 0.543174, 0.544639, 0.546102, 0.547563, 0.549023, 0.550481, 0.551937, 0.553392, 0.554844, 0.556296, 0.557745, 0.559193, 0.560639, 0.562083, 0.563526, 0.564967, 0.566406, 0.567844, 0.56928, 0.570714, 0.572146, 0.573576, 0.575005, 0.576432, 0.577858, 0.579281, 0.580703, 0.582123, 0.583541, 0.584958, 0.586372, 0.587785, 0.589196, 0.590606, 0.592013, 0.593419, 0.594823, 0.596225, 0.597625, 0.599024, 0.60042, 0.601815, 0.603208, 0.604599, 0.605988, 0.607376, 0.608761, 0.610145, 0.611527, 0.612907, 0.614285, 0.615661, 0.617036, 0.618408, 0.619779, 0.621148, 0.622515, 0.62388, 0.625243, 0.626604, 0.627963, 0.62932, 0.630676, 0.632029, 0.633381, 0.634731, 0.636078, 0.637424, 0.638768, 0.64011, 0.64145, 0.642788, 0.644124, 0.645458, 0.64679, 0.64812, 0.649448, 0.650774, 0.652098, 0.653421, 0.654741, 0.656059, 0.657375, 0.658689, 0.660002, 0.661312, 0.66262, 0.663926, 0.66523, 0.666532, 0.667833, 0.669131, 0.670427, 0.671721, 0.673013, 0.674302, 0.67559, 0.676876, 0.67816, 0.679441, 0.680721, 0.681998, 0.683274, 0.684547, 0.685818, 0.687088, 0.688355, 0.68962, 0.690882, 0.692143, 0.693402, 0.694658, 0.695913, 0.697165, 0.698415, 0.699663, 0.700909, 0.702153, 0.703395, 0.704634, 0.705872, 0.707107, 0.70834, 0.709571, 0.710799, 0.712026, 0.71325, 0.714473, 0.715693, 0.716911, 0.718126, 0.71934, 0.720551, 0.72176, 0.722967, 0.724172, 0.725374, 0.726575, 0.727773, 0.728969, 0.730162, 0.731354, 0.732543, 0.73373, 0.734915, 0.736097, 0.737277, 0.738455, 0.739631, 0.740805, 0.741976, 0.743145, 0.744312, 0.745476, 0.746638, 0.747798, 0.748956, 0.750111, 0.751264, 0.752415, 0.753563, 0.75471, 0.755853, 0.756995, 0.758134, 0.759271, 0.760406, 0.761538, 0.762668, 0.763796, 0.764921, 0.766044, 0.767165, 0.768284, 0.7694, 0.770513, 0.771625, 0.772734, 0.77384, 0.774944, 0.776046, 0.777146, 0.778243, 0.779338, 0.78043, 0.78152, 0.782608, 0.783693, 0.784776, 0.785857, 0.786935, 0.788011, 0.789084, 0.790155, 0.791224, 0.79229, 0.793353, 0.794415, 0.795473, 0.79653, 0.797584, 0.798636, 0.799685, 0.800731, 0.801776, 0.802817, 0.803857, 0.804894, 0.805928, 0.80696, 0.80799, 0.809017, 0.810042, 0.811064, 0.812084, 0.813101, 0.814116, 0.815128, 0.816138, 0.817145, 0.81815, 0.819152, 0.820152, 0.821149, 0.822144, 0.823136, 0.824126, 0.825113, 0.826098, 0.827081, 0.82806, 0.829038, 0.830012, 0.830984, 0.831954, 0.832921, 0.833886, 0.834848, 0.835807, 0.836764, 0.837719, 0.838671, 0.83962, 0.840567, 0.841511, 0.842452, 0.843391, 0.844328, 0.845262, 0.846193, 0.847122, 0.848048, 0.848972, 0.849893, 0.850811, 0.851727, 0.85264, 0.853551, 0.854459, 0.855364, 0.856267, 0.857167, 0.858065, 0.85896, 0.859852, 0.860742, 0.861629, 0.862514, 0.863396, 0.864275, 0.865151, 0.866025, 0.866897, 0.867765, 0.868632, 0.869495, 0.870356, 0.871214, 0.872069, 0.872922, 0.873772, 0.87462, 0.875465, 0.876307, 0.877146, 0.877983, 0.878817, 0.879649, 0.880477, 0.881303, 0.882127, 0.882948, 0.883766, 0.884581, 0.885394, 0.886204, 0.887011, 0.887815, 0.888617, 0.889416, 0.890213, 0.891007, 0.891798, 0.892586, 0.893371, 0.894154, 0.894934, 0.895712, 0.896486, 0.897258, 0.898028, 0.898794, 0.899558, 0.900319, 0.901077, 0.901833, 0.902585, 0.903335, 0.904083, 0.904827, 0.905569, 0.906308, 0.907044, 0.907777, 0.908508, 0.909236, 0.909961, 0.910684, 0.911403, 0.91212, 0.912834, 0.913545, 0.914254, 0.91496, 0.915663, 0.916363, 0.91706, 0.917755, 0.918446, 0.919135, 0.919821, 0.920505, 0.921185, 0.921863, 0.922538, 0.92321, 0.92388, 0.924546, 0.92521, 0.925871, 0.926529, 0.927184, 0.927836, 0.928486, 0.929133, 0.929776, 0.930418, 0.931056, 0.931691, 0.932324, 0.932954, 0.93358, 0.934204, 0.934826, 0.935444, 0.93606, 0.936672, 0.937282, 0.937889, 0.938493, 0.939094, 0.939693, 0.940288, 0.940881, 0.941471, 0.942057, 0.942641, 0.943223, 0.943801, 0.944376, 0.944949, 0.945519, 0.946085, 0.946649, 0.94721, 0.947768, 0.948324, 0.948876, 0.949425, 0.949972, 0.950516, 0.951057, 0.951594, 0.952129, 0.952661, 0.953191, 0.953717, 0.95424, 0.954761, 0.955278, 0.955793, 0.956305, 0.956814, 0.957319, 0.957822, 0.958323, 0.95882, 0.959314, 0.959805, 0.960294, 0.960779, 0.961262, 0.961741, 0.962218, 0.962692, 0.963163, 0.96363, 0.964095, 0.964557, 0.965016, 0.965473, 0.965926, 0.966376, 0.966823, 0.967268, 0.967709, 0.968148, 0.968583, 0.969016, 0.969445, 0.969872, 0.970296, 0.970716, 0.971134, 0.971549, 0.971961, 0.97237, 0.972776, 0.973179, 0.973579, 0.973976, 0.97437, 0.974761, 0.975149, 0.975535, 0.975917, 0.976296, 0.976672, 0.977046, 0.977416, 0.977783, 0.978148, 0.978509, 0.978867, 0.979223, 0.979575, 0.979925, 0.980271, 0.980615, 0.980955, 0.981293, 0.981627, 0.981959, 0.982287, 0.982613, 0.982935, 0.983255, 0.983571, 0.983885, 0.984196, 0.984503, 0.984808, 0.985109, 0.985408, 0.985703, 0.985996, 0.986286, 0.986572, 0.986856, 0.987136, 0.987414, 0.987688, 0.98796, 0.988228, 0.988494, 0.988756, 0.989016, 0.989272, 0.989526, 0.989776, 0.990024, 0.990268, 0.990509, 0.990748, 0.990983, 0.991216, 0.991445, 0.991671, 0.991894, 0.992115, 0.992332, 0.992546, 0.992757, 0.992966, 0.993171, 0.993373, 0.993572, 0.993768, 0.993961, 0.994151, 0.994338, 0.994522, 0.994703, 0.994881, 0.995056, 0.995227, 0.995396, 0.995562, 0.995725, 0.995884, 0.996041, 0.996195, 0.996345, 0.996493, 0.996637, 0.996779, 0.996917, 0.997053, 0.997185, 0.997314, 0.997441, 0.997564, 0.997684, 0.997801, 0.997916, 0.998027, 0.998135, 0.99824, 0.998342, 0.998441, 0.998537, 0.99863, 0.998719, 0.998806, 0.99889, 0.998971, 0.999048, 0.999123, 0.999194, 0.999263, 0.999328, 0.999391, 0.99945, 0.999507, 0.99956, 0.99961, 0.999657, 0.999701, 0.999743, 0.999781, 0.999816, 0.999848, 0.999877, 0.999903, 0.999925, 0.999945, 0.999962, 0.999976, 0.999986, 0.999994, 0.999998};
float cos_table[900] = {1, 0.999998, 0.999994, 0.999986, 0.999976, 0.999962, 0.999945, 0.999925, 0.999903, 0.999877, 0.999848, 0.999816, 0.999781, 0.999743, 0.999701, 0.999657, 0.99961, 0.99956, 0.999507, 0.99945, 0.999391, 0.999328, 0.999263, 0.999194, 0.999123, 0.999048, 0.998971, 0.99889, 0.998806, 0.998719, 0.99863, 0.998537, 0.998441, 0.998342, 0.99824, 0.998135, 0.998027, 0.997916, 0.997801, 0.997684, 0.997564, 0.997441, 0.997314, 0.997185, 0.997053, 0.996917, 0.996779, 0.996637, 0.996493, 0.996345, 0.996195, 0.996041, 0.995884, 0.995725, 0.995562, 0.995396, 0.995227, 0.995056, 0.994881, 0.994703, 0.994522, 0.994338, 0.994151, 0.993961, 0.993768, 0.993572, 0.993373, 0.993171, 0.992966, 0.992757, 0.992546, 0.992332, 0.992115, 0.991894, 0.991671, 0.991445, 0.991216, 0.990983, 0.990748, 0.990509, 0.990268, 0.990024, 0.989776, 0.989526, 0.989272, 0.989016, 0.988756, 0.988494, 0.988228, 0.98796, 0.987688, 0.987414, 0.987136, 0.986856, 0.986572, 0.986286, 0.985996, 0.985703, 0.985408, 0.985109, 0.984808, 0.984503, 0.984196, 0.983885, 0.983571, 0.983255, 0.982935, 0.982613, 0.982287, 0.981959, 0.981627, 0.981293, 0.980955, 0.980615, 0.980271, 0.979925, 0.979575, 0.979223, 0.978867, 0.978509, 0.978148, 0.977783, 0.977416, 0.977046, 0.976672, 0.976296, 0.975917, 0.975535, 0.975149, 0.974761, 0.97437, 0.973976, 0.973579, 0.973179, 0.972776, 0.97237, 0.971961, 0.971549, 0.971134, 0.970716, 0.970296, 0.969872, 0.969445, 0.969016, 0.968583, 0.968148, 0.967709, 0.967268, 0.966823, 0.966376, 0.965926, 0.965473, 0.965016, 0.964557, 0.964095, 0.96363, 0.963163, 0.962692, 0.962218, 0.961741, 0.961262, 0.960779, 0.960294, 0.959805, 0.959314, 0.95882, 0.958323, 0.957822, 0.957319, 0.956814, 0.956305, 0.955793, 0.955278, 0.954761, 0.95424, 0.953717, 0.953191, 0.952661, 0.952129, 0.951594, 0.951057, 0.950516, 0.949972, 0.949425, 0.948876, 0.948324, 0.947768, 0.94721, 0.946649, 0.946085, 0.945519, 0.944949, 0.944376, 0.943801, 0.943223, 0.942641, 0.942057, 0.941471, 0.940881, 0.940288, 0.939693, 0.939094, 0.938493, 0.937889, 0.937282, 0.936672, 0.93606, 0.935444, 0.934826, 0.934204, 0.93358, 0.932954, 0.932324, 0.931691, 0.931056, 0.930418, 0.929776, 0.929133, 0.928486, 0.927836, 0.927184, 0.926529, 0.925871, 0.92521, 0.924546, 0.92388, 0.92321, 0.922538, 0.921863, 0.921185, 0.920505, 0.919821, 0.919135, 0.918446, 0.917755, 0.91706, 0.916363, 0.915663, 0.91496, 0.914254, 0.913545, 0.912834, 0.91212, 0.911403, 0.910684, 0.909961, 0.909236, 0.908508, 0.907777, 0.907044, 0.906308, 0.905569, 0.904827, 0.904083, 0.903335, 0.902585, 0.901833, 0.901077, 0.900319, 0.899558, 0.898794, 0.898028, 0.897258, 0.896486, 0.895712, 0.894934, 0.894154, 0.893371, 0.892586, 0.891798, 0.891007, 0.890213, 0.889416, 0.888617, 0.887815, 0.887011, 0.886204, 0.885394, 0.884581, 0.883766, 0.882948, 0.882127, 0.881303, 0.880477, 0.879649, 0.878817, 0.877983, 0.877146, 0.876307, 0.875465, 0.87462, 0.873772, 0.872922, 0.872069, 0.871214, 0.870356, 0.869495, 0.868632, 0.867765, 0.866897, 0.866025, 0.865151, 0.864275, 0.863396, 0.862514, 0.861629, 0.860742, 0.859852, 0.85896, 0.858065, 0.857167, 0.856267, 0.855364, 0.854459, 0.853551, 0.85264, 0.851727, 0.850811, 0.849893, 0.848972, 0.848048, 0.847122, 0.846193, 0.845262, 0.844328, 0.843391, 0.842452, 0.841511, 0.840567, 0.83962, 0.838671, 0.837719, 0.836764, 0.835807, 0.834848, 0.833886, 0.832921, 0.831954, 0.830984, 0.830012, 0.829038, 0.82806, 0.827081, 0.826098, 0.825113, 0.824126, 0.823136, 0.822144, 0.821149, 0.820152, 0.819152, 0.81815, 0.817145, 0.816138, 0.815128, 0.814116, 0.813101, 0.812084, 0.811064, 0.810042, 0.809017, 0.80799, 0.80696, 0.805928, 0.804894, 0.803857, 0.802817, 0.801776, 0.800731, 0.799685, 0.798636, 0.797584, 0.79653, 0.795473, 0.794415, 0.793353, 0.79229, 0.791224, 0.790155, 0.789084, 0.788011, 0.786935, 0.785857, 0.784776, 0.783693, 0.782608, 0.78152, 0.78043, 0.779338, 0.778243, 0.777146, 0.776046, 0.774944, 0.77384, 0.772734, 0.771625, 0.770513, 0.7694, 0.768284, 0.767165, 0.766044, 0.764921, 0.763796, 0.762668, 0.761538, 0.760406, 0.759271, 0.758134, 0.756995, 0.755853, 0.75471, 0.753563, 0.752415, 0.751264, 0.750111, 0.748956, 0.747798, 0.746638, 0.745476, 0.744312, 0.743145, 0.741976, 0.740805, 0.739631, 0.738455, 0.737277, 0.736097, 0.734915, 0.73373, 0.732543, 0.731354, 0.730162, 0.728969, 0.727773, 0.726575, 0.725374, 0.724172, 0.722967, 0.72176, 0.720551, 0.71934, 0.718126, 0.716911, 0.715693, 0.714473, 0.71325, 0.712026, 0.710799, 0.709571, 0.70834, 0.707107, 0.705872, 0.704634, 0.703395, 0.702153, 0.700909, 0.699663, 0.698415, 0.697165, 0.695913, 0.694658, 0.693402, 0.692143, 0.690882, 0.68962, 0.688355, 0.687088, 0.685818, 0.684547, 0.683274, 0.681998, 0.680721, 0.679441, 0.67816, 0.676876, 0.67559, 0.674302, 0.673013, 0.671721, 0.670427, 0.669131, 0.667833, 0.666532, 0.66523, 0.663926, 0.66262, 0.661312, 0.660002, 0.658689, 0.657375, 0.656059, 0.654741, 0.653421, 0.652098, 0.650774, 0.649448, 0.64812, 0.64679, 0.645458, 0.644124, 0.642788, 0.64145, 0.64011, 0.638768, 0.637424, 0.636078, 0.634731, 0.633381, 0.632029, 0.630676, 0.62932, 0.627963, 0.626604, 0.625243, 0.62388, 0.622515, 0.621148, 0.619779, 0.618408, 0.617036, 0.615661, 0.614285, 0.612907, 0.611527, 0.610145, 0.608761, 0.607376, 0.605988, 0.604599, 0.603208, 0.601815, 0.60042, 0.599024, 0.597625, 0.596225, 0.594823, 0.593419, 0.592013, 0.590606, 0.589196, 0.587785, 0.586372, 0.584958, 0.583541, 0.582123, 0.580703, 0.579281, 0.577858, 0.576432, 0.575005, 0.573576, 0.572146, 0.570714, 0.56928, 0.567844, 0.566406, 0.564967, 0.563526, 0.562083, 0.560639, 0.559193, 0.557745, 0.556296, 0.554844, 0.553392, 0.551937, 0.550481, 0.549023, 0.547563, 0.546102, 0.544639, 0.543174, 0.541708, 0.54024, 0.538771, 0.5373, 0.535827, 0.534352, 0.532876, 0.531399, 0.529919, 0.528438, 0.526956, 0.525472, 0.523986, 0.522499, 0.52101, 0.519519, 0.518027, 0.516533, 0.515038, 0.513541, 0.512043, 0.510543, 0.509041, 0.507538, 0.506034, 0.504528, 0.50302, 0.501511, 0.5, 0.498488, 0.496974, 0.495459, 0.493942, 0.492424, 0.490904, 0.489382, 0.48786, 0.486335, 0.48481, 0.483282, 0.481754, 0.480223, 0.478692, 0.477159, 0.475624, 0.474088, 0.472551, 0.471012, 0.469472, 0.46793, 0.466387, 0.464842, 0.463296, 0.461749, 0.4602, 0.45865, 0.457098, 0.455545, 0.45399, 0.452435, 0.450878, 0.449319, 0.447759, 0.446198, 0.444635, 0.443071, 0.441506, 0.439939, 0.438371, 0.436802, 0.435231, 0.433659, 0.432086, 0.430511, 0.428935, 0.427358, 0.425779, 0.424199, 0.422618, 0.421036, 0.419452, 0.417867, 0.416281, 0.414693, 0.413104, 0.411514, 0.409923, 0.40833, 0.406737, 0.405142, 0.403545, 0.401948, 0.400349, 0.398749, 0.397148, 0.395546, 0.393942, 0.392337, 0.390731, 0.389124, 0.387516, 0.385906, 0.384295, 0.382683, 0.38107, 0.379456, 0.377841, 0.376224, 0.374607, 0.372988, 0.371368, 0.369747, 0.368125, 0.366501, 0.364877, 0.363251, 0.361625, 0.359997, 0.358368, 0.356738, 0.355107, 0.353475, 0.351842, 0.350207, 0.348572, 0.346936, 0.345298, 0.34366, 0.34202, 0.34038, 0.338738, 0.337095, 0.335452, 0.333807, 0.332161, 0.330514, 0.328867, 0.327218, 0.325568, 0.323917, 0.322266, 0.320613, 0.318959, 0.317305, 0.315649, 0.313992, 0.312335, 0.310676, 0.309017, 0.307357, 0.305695, 0.304033, 0.30237, 0.300706, 0.299041, 0.297375, 0.295708, 0.29404, 0.292372, 0.290702, 0.289032, 0.287361, 0.285688, 0.284015, 0.282341, 0.280667, 0.278991, 0.277315, 0.275637, 0.273959, 0.27228, 0.2706, 0.26892, 0.267238, 0.265556, 0.263873, 0.262189, 0.260505, 0.258819, 0.257133, 0.255446, 0.253758, 0.252069, 0.25038, 0.24869, 0.246999, 0.245307, 0.243615, 0.241922, 0.240228, 0.238533, 0.236838, 0.235142, 0.233445, 0.231748, 0.23005, 0.228351, 0.226651, 0.224951, 0.22325, 0.221548, 0.219846, 0.218143, 0.21644, 0.214735, 0.21303, 0.211325, 0.209619, 0.207912, 0.206204, 0.204496, 0.202787, 0.201078, 0.199368, 0.197657, 0.195946, 0.194234, 0.192522, 0.190809, 0.189095, 0.187381, 0.185667, 0.183951, 0.182236, 0.180519, 0.178802, 0.177085, 0.175367, 0.173648, 0.171929, 0.170209, 0.168489, 0.166769, 0.165048, 0.163326, 0.161604, 0.159881, 0.158158, 0.156434, 0.15471, 0.152986, 0.151261, 0.149535, 0.147809, 0.146083, 0.144356, 0.142629, 0.140901, 0.139173, 0.137445, 0.135716, 0.133986, 0.132256, 0.130526, 0.128796, 0.127065, 0.125333, 0.123601, 0.121869, 0.120137, 0.118404, 0.116671, 0.114937, 0.113203, 0.111469, 0.109734, 0.107999, 0.106264, 0.104528, 0.102793, 0.101056, 0.0993197, 0.0975829, 0.0958458, 0.0941083, 0.0923706, 0.0906326, 0.0888943, 0.0871557, 0.0854169, 0.0836778, 0.0819385, 0.0801989, 0.0784591, 0.076719, 0.0749787, 0.0732382, 0.0714974, 0.0697565, 0.0680153, 0.0662739, 0.0645323, 0.0627905, 0.0610485, 0.0593064, 0.057564, 0.0558215, 0.0540788, 0.052336, 0.0505929, 0.0488498, 0.0471065, 0.045363, 0.0436194, 0.0418757, 0.0401318, 0.0383878, 0.0366437, 0.0348995, 0.0331552, 0.0314108, 0.0296662, 0.0279216, 0.0261769, 0.0244322, 0.0226873, 0.0209424, 0.0191974, 0.0174524, 0.0157073, 0.0139622, 0.012217, 0.0104718, 0.00872654, 0.00698126, 0.00523596, 0.00349065, 0.00174533};
// sin and cos function
// input range -3600 to 3600
float sin_val(int16_t theta)
{
	float result = 0;
	theta %= 3600;
	int16_t index = 1;
	if (theta < 0)
	{
		index = -1;
	}
	theta = (theta > 0) ? theta : -theta;
	if (theta < 900)
	{
		result = sin_table[theta];
	}
	else if (theta < 1800)
	{
		result = sin_table[1800 - theta];
	}
	else if (theta < 2700)
	{
		result = - sin_table[theta - 1800];
	}
	else result = - sin_table[3600 - theta];
	result *= index;
	return result;
}

float cos_val(int16_t theta)
{
	float result = 0;
	theta %= 3600;
	theta = (theta > 0) ? theta : -theta;
	if (theta < 900)
	{
		result = cos_table[theta];
	}
	else if (theta < 1800)
	{
		result = - cos_table[1800 - theta];
	}
	else if (theta < 2700)
	{
		result = - cos_table[theta - 1800];
	}
	else result = cos_table[3600 - theta];
	return result;
}

void input_init_all(void) 
{
	input_init_ch();
	input_init_mouse();
}

void input_init_ch(void)
{
	for (int i = 0; i < 4; i++)
	{
		last_ch_input[i] = ch_input[i] = 0;
	}
}

void input_init_mouse(void)
{
	for (int i = 0; i < 2; i++)
	{
		last_mouse_input[i] = mouse_input[i] = 0;
	}
}
