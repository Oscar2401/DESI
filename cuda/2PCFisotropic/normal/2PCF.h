
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <omp.h>

struct Point3D{
	float x;
	float y; 
	float z;
};
struct PointW3D{
	float x;
	float y; 
	float z;
	float w;
};
struct Node{
	Point3D nodepos;	// Coordenadas del nodo (posición del nodo).
	int len;		// Cantidad de elementos en el nodo.
	PointW3D *elements;	// Elementos del nodo.
};

//=================================================================== 
//======================== Clase ==================================== 
//=================================================================== 

class NODE2P{
	//Atributos de clase:
	private:
		// Asignados
		int bn;
		int n_pts;
		float size_box;
		float size_node;
		float d_max;
		Node ***nodeD, ***nodeR;
		PointW3D *dataD, *dataR;
		// Derivados
		float ll;
		float dd_max;
		float corr;
		float front;
		float ds;
		float ddmax_nod;
		
	private: 
		void make_nodos(Node ***, PointW3D *);
		void add(PointW3D *&, int&, float, float, float, float);
	
	// Métodos de Clase:
	public:
		//Constructor de clase:
		NODE2P(int _bn, int _n_pts, float _size_box, float _size_node, float _d_max, PointW3D *_dataD, Node ***_nodeD, PointW3D *_dataR, Node ***_nodeR){
			
			// Asignados
			bn = _bn;
			n_pts = _n_pts;
			size_box = _size_box;
			size_node = _size_node;
			d_max = _d_max;
			dataD = _dataD;
			nodeD = _nodeD;
			dataR = _dataR;
			nodeR = _nodeR;
			
			// Derivados
			dd_max = d_max*d_max;
			front = size_box - d_max;
			corr = size_node*sqrt(3);
			ds = ((float)(bn))/d_max;
			ddmax_nod = d_max+corr;
			ddmax_nod *= ddmax_nod; 
			
			make_nodos(nodeD,dataD); 
			make_nodos(nodeR,dataR); 
			std::cout << "Terminé de contruir nodos..." << std::endl;
		}
		
		Node ***meshData(){
			return nodeD;
		};
		Node ***meshRand(){
			return nodeR;
		};
		
		// Implementamos Método de mallas:
		void make_histoXX(float *, Node ***);
		void make_histoXY(float *, Node ***, Node ***);
		~NODE2P();
};

//=================================================================== 
//==================== Funciones ==================================== 
//===================================================================  

void NODE2P::make_nodos(Node ***nod, PointW3D *dat){
	/*
	Función para crear los nodos con los datos y puntos random
	
	Argumentos
	nod: arreglo donde se crean los nodos.
	dat: datos a dividir en nodos.
	
	*/
	int i, row, col, mom, partitions = (int)((size_box/size_node)+1);
	float p_med = size_node/2;
	
	// Inicializamos los nodos vacíos:
	for (row=0; row<partitions; row++){
	for (col=0; col<partitions; col++){
	for (mom=0; mom<partitions; mom++){
		nod[row][col][mom].nodepos.z = ((float)(mom)*(size_node))+p_med;
		nod[row][col][mom].nodepos.y = ((float)(col)*(size_node))+p_med;
		nod[row][col][mom].nodepos.x = ((float)(row)*(size_node))+p_med;
		nod[row][col][mom].len = 0;
		nod[row][col][mom].elements = new PointW3D[0];
	}
	}
	}
	// Llenamos los nodos con los puntos de dat:
	for (i=0; i<n_pts; ++i){
		row = (int)(dat[i].x/size_node);
        	col = (int)(dat[i].y/size_node);
        	mom = (int)(dat[i].z/size_node);
		add( nod[row][col][mom].elements, nod[row][col][mom].len, dat[i].x, dat[i].y, dat[i].z, dat[i].w);
	}
}

//=================================================================== 

void NODE2P::add(PointW3D *&array, int &lon, float _x, float _y, float _z, float _w){
	lon++;
	PointW3D *array_aux = new PointW3D[lon];
	for (int i=0; i<lon-1; i++){
		array_aux[i].x = array[i].x;
		array_aux[i].y = array[i].y;
		array_aux[i].z = array[i].z;
		array_aux[i].w = array[i].w;
	}
	delete[] array;
	array = array_aux;
	array[lon-1].x = _x;
	array[lon-1].y = _y; 
	array[lon-1].z = _z;
	array[lon-1].w = _w; 
}

//=================================================================== 

void NODE2P::make_histoXX(float *XX, Node ***nodeX){
	/*
	Función para crear los histogramas DD y RR.
	
	Argumentos
	DD: arreglo donde se creará el histograma DD.
	RR: arreglo donde se creará el histograma RR.
	
	*/
	
	int partitions = (int)((size_box/size_node)+1);
	int i, j, row, col, mom, u, v, w;
	float dis, dis_nod;
	float x1D, y1D, z1D, x2D, y2D, z2D;
	float x, y, z, w1;
	float dx, dy, dz, dx_nod, dy_nod, dz_nod;
	bool con_x, con_y, con_z;
	
	std::cout << "-> Estoy haciendo histograma XX..." << std::endl;
	
	for (row = 0; row < partitions; ++row){
	x1D = nodeX[row][0][0].nodepos.x;
	for (col = 0; col < partitions; ++col){
	y1D = nodeX[row][col][0].nodepos.y;
	for (mom = 0; mom < partitions; ++mom){
	z1D = nodeX[row][col][mom].nodepos.z;			
		//==================================================
		// Distancias entre puntos del mismo nodo:
		//==================================================
		for ( i= 0; i <nodeX[row][col][mom].len - 1; ++i){
		x = nodeX[row][col][mom].elements[i].x;
		y = nodeX[row][col][mom].elements[i].y;
		z = nodeX[row][col][mom].elements[i].z;
		w1 = nodeX[row][col][mom].elements[i].w;
			for ( j = i+1; j < nodeX[row][col][mom].len; ++j){
			dx = x-nodeX[row][col][mom].elements[j].x;
			dy = y-nodeX[row][col][mom].elements[j].y;
			dz = z-nodeX[row][col][mom].elements[j].z;
			dis = dx*dx+dy*dy+dz*dz;
			if (dis <= dd_max){
			*(XX + (int)(sqrt(dis)*ds)) += 2*w1*nodeX[row][col][mom].elements[j].w;
			}
			}
		}
		//==================================================
		// Distancias entre puntos del diferente nodo:
		//==================================================
		u = row;
		v = col;
		//=========================
		// N2 movil en Z
		//=========================
		for (w=mom+1;  w<partitions ; ++w){	
		z2D = nodeX[u][v][w].nodepos.z;
		dz_nod = z1D-z2D;
		dis_nod = dz_nod*dz_nod;
		if (dis_nod <= ddmax_nod){
			for ( i = 0; i < nodeX[row][col][mom].len; ++i){
			x = nodeX[row][col][mom].elements[i].x;
			y = nodeX[row][col][mom].elements[i].y;
			z = nodeX[row][col][mom].elements[i].z;
			w1 = nodeX[row][col][mom].elements[i].w;
				for ( j = 0; j < nodeX[u][v][w].len; ++j){
				dx = x-nodeX[u][v][w].elements[j].x;
				dy = y-nodeX[u][v][w].elements[j].y;
				dz = z-nodeX[u][v][w].elements[j].z;
				dis = dx*dx+dy*dy+dz*dz;
				if (dis <= dd_max){
				*(XX + (int)(sqrt(dis)*ds)) += 2*w1*nodeX[u][v][w].elements[j].w;
				}
				}
				}
			}
		}
		//=========================
		// N2 movil en ZY
		//=========================
		for (v = col + 1; v < partitions ; ++v){
		y2D = nodeX[u][v][0].nodepos.y;
		dy_nod = y1D-y2D;
		dy_nod *= dy_nod;
			for (w = 0; w < partitions ; ++w){		
			z2D = nodeX[u][v][w].nodepos.z;
			dz_nod = z1D-z2D;
			dz_nod *= dz_nod;
			dis_nod = dy_nod + dz_nod;
			if (dis_nod <= ddmax_nod){
				for ( i = 0; i < nodeX[row][col][mom].len; ++i){
				x = nodeX[row][col][mom].elements[i].x;
				y = nodeX[row][col][mom].elements[i].y;
				z = nodeX[row][col][mom].elements[i].z;
				w1 = nodeX[row][col][mom].elements[i].w;
					for ( j = 0; j < nodeX[u][v][w].len; ++j){	
					dx =  x-nodeX[u][v][w].elements[j].x;
					dy =  y-nodeX[u][v][w].elements[j].y;
					dz =  z-nodeX[u][v][w].elements[j].z;
					dis = dx*dx+dy*dy+dz*dz;
					if (dis <= dd_max){
					*(XX + (int)(sqrt(dis)*ds)) += 2*w1*nodeX[u][v][w].elements[j].w;
					}
					}
				}
			}
			}
		}
		//=========================
		// N2 movil en ZYX
		//=========================
		for ( u = row + 1; u < partitions; ++u){
		x2D = nodeX[u][0][0].nodepos.x;
		dx_nod = x1D-x2D;
		dx_nod *= dx_nod;	
			for ( v = 0; v < partitions; ++v){
			y2D = nodeX[u][v][0].nodepos.y;
			dy_nod = y1D-y2D;
			dy_nod *= dy_nod;	
				for ( w = 0; w < partitions; ++w){
				z2D = nodeX[u][v][w].nodepos.z;
				dz_nod = z1D-z2D;
				dz_nod *= dz_nod;
				dis_nod = dx_nod + dy_nod + dz_nod;
				if (dis_nod <= ddmax_nod){
					for ( i = 0; i < nodeX[row][col][mom].len; ++i){
					x = nodeX[row][col][mom].elements[i].x;
					y = nodeX[row][col][mom].elements[i].y;
					z = nodeX[row][col][mom].elements[i].z;
					w1 = nodeX[row][col][mom].elements[i].w;
						for ( j = 0; j < nodeX[u][v][w].len; ++j){	
						dx = x-nodeX[u][v][w].elements[j].x;
						dy = y-nodeX[u][v][w].elements[j].y;
						dz = z-nodeX[u][v][w].elements[j].z;
						dis = dx*dx + dy*dy + dz*dz;
						if (dis <= dd_max){
							*(XX + (int)(sqrt(dis)*ds)) += 2*w1*nodeX[u][v][w].elements[j].w;
						}
						}
					}
					}	
				}	
			}
		}
	}
	}
	}
}
//=================================================================== 

void NODE2P::make_histoXY(float *XY, Node ***nodeX, Node ***nodeY){
	/*
	Función para crear los histogramas DR.
	
	Argumentos
	XY: arreglo donde se creará el histograma DR.
	
	*/
	
	int partitions = (int)((size_box/size_node)+1);
	int i, j, row, col, mom, u, v, w;
	float dis, dis_nod;
	float x1D, y1D, z1D, x2R, y2R, z2R;
	float x, y, z, w1;
	float dx, dy, dz, dx_nod, dy_nod, dz_nod;
	bool con_x, con_y, con_z;
	
	std::cout << "-> Estoy haciendo histograma XY..." << std::endl;
	
	for (row = 0; row < partitions; ++row){
	x1D = nodeX[row][0][0].nodepos.x;
	for (col = 0; col < partitions; ++col){
	y1D = nodeX[row][col][0].nodepos.y;
	for (mom = 0; mom < partitions; ++mom){
	z1D = nodeX[row][col][mom].nodepos.z;			
	//=========================
	// N2 movil en ZYX
	//=========================
	for (u=0; u<partitions; ++u){
	x2R = nodeY[u][0][0].nodepos.x;
	dx_nod = x1D-x2R;
	dx_nod *= dx_nod;
		for (v=0; v<partitions; ++v){
		y2R = nodeY[u][v][0].nodepos.y;
		dy_nod = y1D-y2R;
		dy_nod *= dy_nod;
			for (w=0; w<partitions; ++w){
			z2R = nodeY[u][v][w].nodepos.z;
			dz_nod = z1D-z2R;
			dz_nod *= dz_nod; 
			dis_nod = dx_nod + dy_nod + dz_nod;
			if (dis_nod <= ddmax_nod){
				for (i=0; i<nodeX[row][col][mom].len; ++i){
				x = nodeX[row][col][mom].elements[i].x;
				y = nodeX[row][col][mom].elements[i].y;
				z = nodeX[row][col][mom].elements[i].z;
				w1 = nodeX[row][col][mom].elements[i].w;
					for (j=0; j<nodeY[u][v][w].len; ++j){	
					dx = x-nodeY[u][v][w].elements[j].x;
					dy = y-nodeY[u][v][w].elements[j].y;
					dz = z-nodeY[u][v][w].elements[j].z;
					dis = dx*dx+dy*dy+dz*dz;
					if (dis < dd_max){
					*(XY+int(sqrt(dis)*ds)) += w1*nodeY[u][v][w].elements[j].w;
					}
					}
				}
			}	
			}	
		}
	}
	}
	}
	}
}
//=================================================================== 

NODE2P::~NODE2P(){
	
}
