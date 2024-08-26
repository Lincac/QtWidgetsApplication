__kernel void Func(
    __global const unsigned char* input,
    __constant int* planeParam,
    __global float* vboMem,
    __global int* vertices,
    __constant int* edgeTable, 
    __constant int* vertTable
    )
{
	int id_x = get_global_id(0); 
	int id_y = get_global_id(1);
	int id_z = get_global_id(2); 

	int width = get_global_size(0); 
    int height = get_global_size(1); 
    int depth = get_global_size(2); 
    
	if (id_x >= width - 2) return;
	if (id_y >= height - 2) return;
	if (id_z >= depth - 2) return;

	int D = planeParam[0] * id_x + planeParam[1] * id_y + planeParam[2] * id_z + planeParam[3];
	if(D != 0) return;    

	float3 pos = (float3)(id_x,id_y,id_z);

	int2 edgeConnection[4] =
	{
		(int2)(0,1), (int2)(1,2), (int2)(2,3), (int2)(3,0),
	};    

	float3 vertexOffset[4];
    vertexOffset[0] = (float3)(0.0f,0.0f,0.0f);
	float3 edgeDirection[4];  

	float quad[4];
	quad[0] = (float)input[id_x + id_y * width + id_z * width * height];
  
	if(planeParam[0] == 1) // ZY
	{
		quad[1] = (float)input[id_x + id_y * width + (id_z + 1) * width * height];
		quad[2] = (float)input[id_x + (id_y + 1) * width + (id_z + 1) * width * height];
		quad[3] = (float)input[id_x + (id_y + 1) * width + id_z * width * height];

		vertexOffset[1] = (float3)(0.0f,0.0f,1.0f);
		vertexOffset[2] = (float3)(0.0f,1.0f,1.0f);
		vertexOffset[3] = (float3)(0.0f,1.0f,0.0f);   

		edgeDirection[0] = (float3)(0.0f, 0.0f, 1.0f);
		edgeDirection[1] = (float3)(0.0f, 1.0f, 0.0f);
		edgeDirection[2] = (float3)(0.0f, 0.0f, -1.0f);
		edgeDirection[3] = (float3)(0.0f, -1.0f, 0.0f);     
	}
	else if(planeParam[1] == 1) // XZ
	{
		quad[1] = (float)input[(id_x + 1) + id_y * width + id_z * width * height];
		quad[2] = (float)input[(id_x + 1) + id_y * width + (id_z + 1) * width * height];
		quad[3] = (float)input[id_x + id_y * width + (id_z + 1) * width * height];

		vertexOffset[1] = (float3)(1.0f,0.0f,0.0f);
		vertexOffset[2] = (float3)(1.0f,0.0f,1.0f);
		vertexOffset[3] = (float3)(0.0f,0.0f,1.0f);

		edgeDirection[0] = (float3)(1.0f, 0.0f, 0.0f);
		edgeDirection[1] = (float3)(0.0f, 0.0f, 1.0f);
		edgeDirection[2] = (float3)(-1.0f, 0.0f, 0.0f);
		edgeDirection[3] = (float3)(0.0f, 0.0f, -1.0f);        
	}
	else if(planeParam[2] == 1) // XY
	{
		quad[1] = (float)input[(id_x + 1) + id_y * width + id_z * width * height];
		quad[2] = (float)input[(id_x + 1) + (id_y + 1) * width + id_z * width * height];
		quad[3] = (float)input[id_x + (id_y + 1) * width + id_z * width * height];

		vertexOffset[1] = (float3)(1.0f,0.0f,0.0f);
		vertexOffset[2] = (float3)(1.0f,1.0f,0.0f);
		vertexOffset[3] = (float3)(0.0f,1.0f,0.0f);        

		edgeDirection[0] = (float3)(1.0f, 0.0f, 0.0f);
		edgeDirection[1] = (float3)(0.0f, 1.0f, 0.0f);
		edgeDirection[2] = (float3)(-1.0f, 0.0f, 0.0f);
		edgeDirection[3] = (float3)(0.0f, -1.0f, 0.0f);        
	}    

	int i = 0;
	int flagIndex = 0;
	for (i = 0; i < 4; i++)
		if (quad[i] >= 255.0) flagIndex |= 1 << i;


	int edgeFlags = edgeTable[flagIndex];
    //printf("OpenCL Kernel Output edgeFlags : %d\n",edgeFlags);
	if (edgeFlags == 0) return; 

	float3 edgeVertex[4];
	for (i = 0; i < 4; i++)
	{
		if ((edgeFlags & (1 << i)) != 0)
		{
			float offset = 0.5;
			edgeVertex[i] = pos + (vertexOffset[edgeConnection[i].x] + offset * edgeDirection[i]);
		}
	}   

	for (i = 0; i < 2; i++)
	{
		if (vertTable[flagIndex * 4 + 2 * i] >= 0)
		{
			int vertexIndex = atomic_add(vertices, 6);
			float3 position;
			position = edgeVertex[vertTable[flagIndex * 4 + (2 * i + 0)]];
			vboMem[vertexIndex + 0] = (float)position.x;
			vboMem[vertexIndex + 1] = (float)position.y;
			vboMem[vertexIndex + 2] = (float)position.z;

            //printf("OpenCL Kernel Output vboMem: %f,%f,%f\n",vboMem[vertexIndex + 0],vboMem[vertexIndex + 1],vboMem[vertexIndex + 2]);

			position = edgeVertex[vertTable[flagIndex * 4 + (2 * i + 1)]];
			vboMem[vertexIndex + 3] = (float)position.x;
			vboMem[vertexIndex + 4] = (float)position.y;
			vboMem[vertexIndex + 5] = (float)position.z;
		}
	}            
}