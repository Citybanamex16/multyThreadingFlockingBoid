/*
Author:Carlos Delgado Contreras 
Matricula: A01712819
Date: 20 de mayo de 2026
*/

#include<thread> //Gestion de los Threads
#include<mutex> // Mecanismos de Exclusion Mutua
#include<condition_variable> //Mecanizmo de notifiacion a threads
#include<functional> // para funciones anonimas y contenedores 
#include<queue> //Estructura de datos para la fila de tasks
#include<vector>
#include <iostream>
#include <cmath> //para al cuadrado en distancias entre agentes
#include <ctime>
#include <cstdlib>
#include <iomanip> //librería para outputs 


using namespace std;

class ThreadPool{
private:
	// ===Atributos===

	//Lista de trabajadores vivos 
	vector<thread> threads_list;


	//Queue de tasks (funciones void)
	std::queue<function<void()>> task_queue;

	//variable de condición para comunicar a todos los thread
	std::condition_variable alarmaThreads;

	//Llave de exclusión mutua
	std::mutex llaveMtx;

	//Booleano de activar/desactivar ThreadPool
	bool poolFinished = false; //false by default

	//cantidad de threads
	int pool_size = 0;

	
public:
	// ===Funciones===
	ThreadPool(size_t workers_amount){
		pool_size = workers_amount;
		//ciclo de creación de workers
		for(size_t i = 0; i < workers_amount; i++){
			threads_list.emplace_back([this]
			{
				// === Funcion infinita de Worker ===
				while(true)
				{
				 std::function<void()> task; //Variable de la tarea a hacer

					{
					//1. Bloquear el queue con RAII para que cada quien tome una task de forma organizada
					std::unique_lock<std::mutex> lock(llaveMtx);

					//2. Esperamos a que nos llamen por el megafono (cv)

					//3. Despertamos
					alarmaThreads.wait(lock,[this] {return !task_queue.empty() || poolFinished; });

					// 3A: ¡Fin del Pool! Debemos irnos
					if(poolFinished && task_queue.empty()){
						//Acabo el pool y no hay mas tareas 
						return; //lock desaparece como la variable local que era
						} 


					// 3B: ¡Notificación recibida! Debemos consultar la queue de tareas
					 task = std::move(task_queue.front()); //move no hace una copia a diferencia de front() :)
        			 task_queue.pop(); //eliminamos la task de la lista 

					} // Fuera de esta llave lock muere pues es una variable local
				
				//4. Nos ponemos a trabajar en task sin lockear a los demas
				task();
 

				}


			});
		}
	}

	~ThreadPool(){
		//1. Bloquear con RAII para actualizar poolFinished y que todos despues la puedan leer.

		{
			std::unique_lock<std::mutex> lock(llaveMtx);
			//2. activar la bandera de fin de pool
			poolFinished = true;

			//3. Avisamos a TODOS los workers que vamos a cerrar
			alarmaThreads.notify_all(); 

		}

		//4. utilizar join() para confirmar que ya acabaron los workers y esperar si no. 
		for (std::thread &worker : threads_list) {
	        if (worker.joinable()) { // Buena práctica: verificar si se puede unir
	            worker.join();
	        }
	    }
	}

	void enqueue(function<void()> newTask){
		//1. bloquear el queue usando RAII para que nadie lo pueda modificar
		{
			std::unique_lock<std::mutex> lock(llaveMtx);
			//2. Insertar la nueva tarea en el queue
			task_queue.emplace(move(newTask));
		}
		//3. Activar la alarma de task -> despertar a los trabajadores
		alarmaThreads.notify_one(); //Solo despertamos a 1 para evitar estres en CPU

	}

	int getSizePool() const {return pool_size;}




};


class Agente{
private:
	//Variables solo de lectura, ningun thread puede modificarlo
	int id;
    float currentX = 0.0; 
    float currentY = 0.0;
    float currentVelocityX = 0.0;
    float currentVelocityY = 0.0; 

    float nextX = 0.0;
    float nextY = 0.0;
    float nextVelocityX = 0.0;
    float nextVelocityY = 0.0;
public: 
	Agente(int _id){

		id = _id;
	}


	void updateAtributes(){
		currentX = nextX;
		currentY = nextY;
		currentVelocityX = nextVelocityX;
		currentVelocityY = nextVelocityY;
	}

	void setNextAttributes(float nx, float ny, float nvx, float nvy) {
    nextX = nx;
    nextY = ny;
    nextVelocityX = nvx;
    nextVelocityY = nvy;
}

	inline int get_id() const{
		return id;
	}

	inline float getX() const { return currentX; }
    inline float getY() const { return currentY; }
    inline float getVX() const { return currentVelocityX; }
    inline float getVY() const { return currentVelocityY; }

    	void initRandom(float maxX, float maxY) {
    // Posiciones aleatorias dentro del espacio virtual
    currentX = nextX = ((float)rand() / RAND_MAX) * maxX;
    currentY = nextY = ((float)rand() / RAND_MAX) * maxY;
    
    // Velocidades iniciales aleatorias pequeñas (entre -2.0 y 2.0 unidades/frame)
    currentVelocityX = nextVelocityX = (((float)rand() / RAND_MAX) * 4.0f) - 2.0f;
    currentVelocityY = nextVelocityY = (((float)rand() / RAND_MAX) * 4.0f) - 2.0f;
}


};



void imprimirAgentes(const std::vector<Agente>& vector_agentes) {
    std::cout << "\n=== ESTADO INICIAL DE LOS AGENTES ===" << std::endl;
    
    // Usamos un ciclo 'for' basado en rangos con referencia constante
    for (const Agente& ag : vector_agentes) {
        std::cout << "Agente ID: " << ag.get_id() 
                  << " | Posicion: (" << ag.getX() << ", " << ag.getY() << ")" 
                  << std::endl;
    }
    std::cout << "======================================\n" << std::endl;
}


//Estructura auxiliar de facil acceso para almacenar vectores 2d (x,y)
struct Vector2D {
    float x = 0.0f;
    float y = 0.0f;
};


int main()
{
	srand(time(NULL)); 
	int agentAmount;
	std::cout<< "¿How many agents you want to create?" << std::endl;
	std::cin >> agentAmount;
	std::cout<< "Creating " << agentAmount << " agents for simulation"<<std::endl;

	//Contenedor de agentes 
	std::vector<Agente> lista_agentes;

	//Instanciamos y guardamos en la lista de agentes todos los agentes
	for(int i = 0; i < agentAmount; i++){
		Agente nuevoAgente(i+1);
		nuevoAgente.initRandom(800.0f, 600.0f); // Espacio virtual de 800x600
    	lista_agentes.emplace_back(nuevoAgente);
	}

	imprimirAgentes(lista_agentes);

	ThreadPool threadpool(5);
	
	//Agentes creados y Threads creados

	for (int frame = 0; frame < 100; frame++){

		//===Setting Fork and Join===
	int inicio = 0;
	int fin = 0;
	int threadAmount = threadpool.getSizePool();


	//0. Setting de Herramientas de Control
	int tareasPendientes = threadAmount;
	std::mutex mainLlave; //encerrar el contador 
	std::condition_variable mainAlarma; //Poder activar y desactivar el thread de main

	//1. Dividir los agentes a los threads (Fork)
	for(int i = 0; i < threadAmount; i++){
		//Calculamos los nuevos Inicio y fin
		inicio = fin; 

		if(i == threadAmount-1){
			//El fin son los restantes
			fin = agentAmount;
		}
		else{
			//Respetamos el paso
		fin += agentAmount/threadAmount; // ej: 1000/4 -> 250 -> 0+250 or 250+250
		}

		//Enqueue de funcion que trabajaran los threads
		threadpool.enqueue([inicio,fin,&tareasPendientes,&mainLlave,&mainAlarma,&lista_agentes](){
			//===Trabajo en paralelo de cada agente===
			for(int i = inicio; i < fin; i++){
				//=== Calculo de fuerzas de raynolds por agente ===

				//1. Inicializar Acumuladores de fuerzas y mi agente
				Vector2D sep; //fuerza de separación
				Vector2D ali; //fuerza de alineación
				Vector2D coh; //fuerza de cohesión
				int cont = 0; //contador de vecinos
				Agente& agenteActual = lista_agentes[i];
				int id = agenteActual.get_id();
				float posX = agenteActual.getX();
				float posY = agenteActual.getY();
				float RadioVision = 100.0f; //¿Que valor sería bueno?
				float RadioVisionCuadrado = RadioVision * RadioVision;
				float RadioSeparacion = RadioVision*0.20; // Radio de colisión extrema
				float RadioSeparacionCuadrado = RadioSeparacion * RadioSeparacion;

				//2. bucle de vecinos
				for(Agente& vecino: lista_agentes)
				{
					int vecinoId = vecino.get_id();
					

					//3. filtro de identidad 
					if(id == vecinoId)
					{
					continue;
					}

					//extracción de datos de vecino
					float vecinoPosY = vecino.getY();
					float vecinoPosX = vecino.getX();

					float dx = vecinoPosX - posX;
			        float dy = vecinoPosY - posY;
			        float distanciaCuadrada = (dx * dx) + (dy * dy);
					//float distanciaCuadrada = pow((vecinoPosX - posX),2) + pow((vecinoPosY- posY),2);

					//4. Filtro de Distancia
					if(distanciaCuadrada < RadioVisionCuadrado){
						cont++;

						// == procesar fuerzas de raynolds ==

						//1. Fuerza de Cohesión (pos de los vecinos)
						coh.x += vecinoPosX;
						coh.y += vecinoPosY;

						//2. Fuerza de Alineación (Velocidades de los vecinos)
						ali.x += vecino.getVX();
						ali.y += vecino.getVY();

						//3. Fuerza de Separación (No chocar)
						if(distanciaCuadrada < RadioSeparacionCuadrado){
							sep.x += (posX - vecinoPosX);
        					sep.y += (posY - vecinoPosY);
						}

					}


				}//Fin de bucle de vecinos

				//4. sacar promedios
				if(cont != 0){
				coh.x /= cont;
				coh.y /= cont;
				ali.x /= cont;
				ali.y /= cont;
				sep.x /= cont;
				sep.y /= cont;
				//5. Calcular el Vector de Dirección hacia el Centro centro - yo
				coh.x = coh.x - posX;
            	coh.y = coh.y - posY;

				}

				//Pesos necesarios para truncar las velocidades y que no salgan volando

				float pesoCohesion = 0.01f;    // Atrae suavemente al grupo
				float pesoAlineacion = 0.05f;  // Imita la velocidad promedio
				float pesoSeparacion = 0.15f;  // Evita colisiones de forma prioritaria

	

				//6. Calcular la Velocidad Futura
				float nuevaVelX = agenteActual.getVX() + (coh.x * pesoCohesion) + (ali.x * pesoAlineacion) + (sep.x * pesoSeparacion);
				float nuevaVelY = agenteActual.getVY() + (coh.y * pesoCohesion) + (ali.y * pesoAlineacion) + (sep.y * pesoSeparacion);
				

				// LIMITAR VELOCIDAD MÁXIMA (un clamp de Godot practicamente)
				// Impide que los agentes aceleren infinitamente 
				float velocidadMaxima = 3.5f; // Unidades máximas por fotograma
				float magnitudVel = sqrt(nuevaVelX * nuevaVelX + nuevaVelY * nuevaVelY); //Checar alternativa mas rapida
				
				if (magnitudVel > velocidadMaxima && magnitudVel > 0.0f) {
					nuevaVelX = (nuevaVelX / magnitudVel) * velocidadMaxima;
					nuevaVelY = (nuevaVelY / magnitudVel) * velocidadMaxima;
				}

				// 7. Calcular la Posición Futura basada en la nueva velocidad 
		        float nuevaPosX = posX + nuevaVelX;
		        float nuevaPosY = posY + nuevaVelY;


				//Efecto espejo. Si se salen de la pantalla salen por el otro lado :)
				if (nuevaPosX < 0.0f) nuevaPosX += 800.0f;
				if (nuevaPosX > 800.0f) nuevaPosX -= 800.0f;
				if (nuevaPosY < 0.0f) nuevaPosY += 600.0f;
				if (nuevaPosY > 600.0f) nuevaPosY -= 600.0f;


				

		        agenteActual.setNextAttributes(nuevaPosX, nuevaPosY, nuevaVelX, nuevaVelY);

			}//Fin de bloque de Agente

			//2. Cerramos con RAII para restar a variable compartida tareasPendientes
			{
			//3. Trabajo terminado -> reportar resultados
			std::unique_lock<std::mutex> lock(mainLlave);
			//restamos a tareas pendientes
			tareasPendientes--;
			} // <- Muere RAII

			//4. Notificamos que hemos terminado :)
			mainAlarma.notify_one();


		});
	}

	//2. Espera de dibujo
	std::unique_lock<std::mutex> lock(mainLlave);
	//Solo me despierto si las tareas estan en 0. Solo aqui es cuando puedo actualizar las pos y velocidades
	mainAlarma.wait(lock, [& tareasPendientes]() { return tareasPendientes == 0; });

	//3. Update masivo en agentes
	for(Agente& agente: lista_agentes){
		agente.updateAtributes();
	}


	// 4. Monitoreo en Consola (Imprimir cómo se mueve el Agente 1 cuadro por cuadro)
        std::cout << std::fixed << std::setprecision(2)
        		  << "Frame: " << frame 
                  << " | Agente 1 Pos: (" << lista_agentes[0].getX() << ", " << lista_agentes[0].getY() << ")"
                  << " | Vel: (" << lista_agentes[0].getVX() << ", " << lista_agentes[0].getVY() << ")" 
                  << std::endl;
                  
        fin = 0; // Resetear el fin para el siguiente cálculo de rebanadas del próximo frame


	}

	
	
	std::cout << "\nSimulacion de prueba completada con exito en paralelo." << std::endl;
	return 0;
}

