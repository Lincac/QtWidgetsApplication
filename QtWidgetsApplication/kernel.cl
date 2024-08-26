__kernel void Func(__global float* vboMem)
{
	int id_x = get_global_id(0); 
	int id_y = get_global_id(1);

	int width = get_global_size(0); 

    vboMem[0] = -0.5;
    vboMem[1] = 0.5;
    vboMem[2] = 0.0;
    vboMem[3] = 0.0;
    vboMem[4] = 1.0;

    vboMem[5] = 0.5;
    vboMem[6] = -0.5;
    vboMem[7] = 0.0;
    vboMem[8] = 1.0;
    vboMem[9] = 0.0;

    vboMem[10] = -0.5;
    vboMem[11] = -0.5;
    vboMem[12] = 0.0;
    vboMem[13] = 0.0;
    vboMem[14] = 0.0;        

    vboMem[15] = -0.5;
    vboMem[16] = 0.5;
    vboMem[17] = 0.0;
    vboMem[18] = 0.0;
    vboMem[19] = 1.0;

    vboMem[20] = 0.5;
    vboMem[21] = 0.5;
    vboMem[22] = 0.0;
    vboMem[23] = 1.0;
    vboMem[24] = 1.0;    

    vboMem[25] = 0.5;
    vboMem[26] = -0.5;
    vboMem[27] = 0.0;
    vboMem[28] = 1.0;
    vboMem[29] = 0.0; 
}