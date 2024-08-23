__kernel void Func(__global float* vboMem)
{
	int id_x = get_global_id(0); 
	int id_y = get_global_id(1);

    if(id_x > 1 || id_y > 5) return;

	int width = get_global_size(0); 
    int index = id_y * 5 + id_x;
    vboMem[index] += 1.0;
}