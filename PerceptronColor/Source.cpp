#include <iostream>
#include <cmath>
#include <ctime>
#include <vector>
#include <memory>
#include <fstream>
#include <algorithm>

using namespace std;

#define ALPHA 0.01f
#define EX_FICH "exemples.txt"
#define NB_ENTREES 3
#define NB_POP 1000
#define GENERATION_MIN 10

enum Fonction{
	HEAVYSIDE, LINEAR, SIGMOIDE
};

enum Color{
	RED = 1, GREEN = 2, BLUE = 3
};

struct Perceptron;

struct Neurone{
	Perceptron* pere;
	float weight[NB_ENTREES];
	float bias;
	float output;
	float(*f)(float, float);
};

struct ColorStr{
	float r, v, b;
	Color out;
};

struct Perceptron{
	float input[NB_ENTREES];
	float output[NB_ENTREES];
	vector<shared_ptr<Neurone>> neurones;
};

struct Individu{
	float W[NB_ENTREES][NB_ENTREES]; //Poids [Neurone][entree]
	float B[NB_ENTREES]; //Biais[Neurone]
	float fitness; //fitness = somme
};

bool heaviside(float sum, float bias){
	return sum >= bias;
}

float linear(float sum, float bias){
	if (sum/255 >= 1){
		return 1;
	}
	else if (sum/255 <= 0){
		return 0;
	}
	else{
		return sum/255;
	}
}

float sigmoide(float sum, float bias){
	float a = expf(-sum / bias);
	return 1 / (1 + a);
}

void initNeurone(Neurone* N, Perceptron* p){
	N->pere = p ;
	N->bias = rand() % 101;
	N->bias /= 100;
	for (int i = 0; i < 3; i++){
		N->weight[i] = rand() % 101;
		N->weight[i] /= 100;
	}
	N->f = linear;
}

void change(Neurone* n){
	for (int i = 0; i < 3; i++){
		n->weight[i] += ALPHA;
		if (n->weight[i] > 1){
			n->weight[i] = 0;
		}
	}
	n->bias += ALPHA;
	if (n->bias > 1){
		n->bias = 0;
	}
}

float eval(Neurone* n){
	float a = 0;
	for (int i = 0; i < NB_ENTREES; i++){
		a += n->pere->input[i] * n->weight[i];
	}
	return a;
}

void initExemples(vector<ColorStr>* ex, ifstream* flx){
	int i = 0;
	int j = 0;
	while (!flx->eof()){
		switch (i)
		{
		case 0:
			ex->resize(ex->size() + 1);
			*flx >> ex->at(j).r;
			break;
		case 1:
			*flx >> ex->at(j).v;
			break;
		case 2:
			*flx >> ex->at(j).b;
			break;
		case 3:
			int k;
			*flx >> k;
			ex->at(j).out = static_cast<Color>(k);
			i = -1;
			j++;
			break;
		default:
			break;
		}
		i++;
	}
}

void appliquerIndivAPerceptron(Perceptron* p, Individu& indiv){
	for (int i = 0; i < p->neurones.size(); i++){
		p->neurones.at(i)->bias = indiv.B[i];
		for (int j = 0; j < NB_ENTREES; j++){
			p->neurones.at(i)->weight[j] = indiv.W[i][j];
		}
	}
	
}

void initInputs(Perceptron* p, vector<ColorStr>& exemples, int nb_essais){
	p->input[0] = exemples.at(nb_essais).r;
	p->input[1] = exemples.at(nb_essais).v;
	p->input[2] = exemples.at(nb_essais).b;
}

void setInputs(Perceptron* p, int r, int g, int b){
	p->input[0] = r;
	p->input[1] = g;
	p->input[2] = b;
}

void calcOutputs(Perceptron* p ){
	for (int u = 0; u < NB_ENTREES; u++){
		float e = eval(p->neurones.at(u).get());
		float s = p->neurones.at(u)->f(e, p->neurones.at(u)->bias);
		p->output[u] = s;
	}
}

int findMax(Perceptron* p){
	int idm = 0;
	for (int r = 1; r < NB_ENTREES; r++){
		if (p->output[r] > p->output[idm]){
			idm = r;
		}
	}
	idm++;
	return idm;
}

void initIndiv(shared_ptr<Individu>* indiv){
	indiv->reset(new Individu());
	for (int i = 0; i < NB_ENTREES; i++){
		for (int j = 0; j < NB_ENTREES; j++){
			indiv->get()->W[i][j] = rand() % 101;
			indiv->get()->W[i][j] /= 100;
		}
		indiv->get()->B[i] = rand() % 101;
		indiv->get()->B[i] /= 100;
	}
	indiv->get()->fitness = -1;
}

void initPop(vector<shared_ptr<Individu>>* pop){
	for (int i = 0; i < pop->size(); i++){
		initIndiv(&pop->at(i));
	}
}

int findBestIndiv(vector<shared_ptr<Individu>>* pop){
	int idmin = 0;
	for (int i = 1; i < pop->size(); i++){
		if (pop->at(idmin)->fitness > pop->at(i)->fitness){
			idmin = i;
		}
	}
	return idmin;
}

void mutation(shared_ptr<Individu>* indiv){
	if (rand() % 2 == 1){
		int x = rand() % NB_ENTREES;
		int y = rand() % NB_ENTREES;
		indiv->get()->W[x][y] = rand() % 101;
		indiv->get()->W[x][y] /= 100;
		
	}
	else{
		int x = rand() % NB_ENTREES;
		indiv->get()->B[x] = rand() % 101;
		indiv->get()->B[x] /= 100;
		
	}
}

void affInfiv(Individu& best){
	for (int i = 0; i < NB_ENTREES; i++){
		for (int j = 0; j < NB_ENTREES; j++){
			cout << best.W[i][j] << " ";
		}
		cout << endl;
	}
	for (int j = 0; j < NB_ENTREES; j++){
		cout << best.B[j] << " ";
	}
	cout << endl;
}

bool trieFonct(shared_ptr<Individu> i1, shared_ptr<Individu> i2){
	return i1->fitness < i2->fitness;
}

void trie(vector<shared_ptr<Individu>>* pop){
	sort(pop->begin(), pop->end(), trieFonct);
}

void selectParents(vector<shared_ptr<Individu>>* pop,pair<int,int>* parents){
	/*int r = rand() % (int(pop->back()->fitness)+1);
	int i = 0;
	while (pop->at(i)->fitness > r && i < NB_POP-1){
		i++;
	}
	//parents->first = i;
	r = rand() % (int(pop->back()->fitness)+1);
	i = 1;
	while (pop->at(i)->fitness > r && i < NB_POP - 1){
		i++;
	}
	//parents->second = i;*/

	parents->first = rand() % (pop->size()/10);
	parents->second = rand() % (pop->size()/10);

}

Individu child(vector<shared_ptr<Individu>>* pop){
	pair<int, int> parents;
	selectParents(pop, &parents);
	Individu child;
	for (int i = 0; i < NB_ENTREES; i++){
		for (int j = 0; j < NB_ENTREES; j++){
			child.W[i][j] = pop->at(parents.first)->W[i][j];
		}
		child.B[i] = pop->at(parents.second)->B[i];
	}
	return child;
}

void newPop(vector<shared_ptr<Individu>>* pop){
	vector<shared_ptr<Individu>> buffer(pop->size());
	for (int i = 0; i < pop->size(); i++){
		buffer.at(i) = shared_ptr<Individu>(new Individu(child(pop)));
		//buffer.at(i)->fitness = pop->at(i)->fitness;
	}
	*pop = buffer;
}

void reinjection(vector<shared_ptr<Individu>>* pop, Individu best){
	int idmax = 0;
	for (int i = 1; i < pop->size(); i++){
		if (pop->at(i)->fitness > pop->at(idmax)->fitness){
			idmax = i;
		}
	}
	pop->at(idmax).reset(new Individu(best));
}

int main(){
	srand(time(NULL));
	vector<ColorStr> exemples;
	ifstream fich(EX_FICH, ios::in);
	if (fich){
		initExemples(&exemples, &fich);
	}
	else{
		return 1;
	}

	vector<shared_ptr<Individu>> population(NB_POP);
	Individu best;
	initPop(&population);
	best = *population.at(0).get();
	best.fitness = 100;
	Perceptron p;
	for (int i = 0; i < 3; i++){
		p.neurones.push_back(shared_ptr<Neurone>(new Neurone));
		initNeurone(p.neurones.at(i).get(),&p);
	}
	int somme = -1;
	int generation = 0;
	int nb_essais = 0;
	while (best.fitness != 0 || generation < GENERATION_MIN){
		//cout <<"GENERATION : " <<generation << endl;
		for (int i = 0; i < population.size(); i++){
			somme = 0;
			nb_essais = 0;
			appliquerIndivAPerceptron(&p, *population.at(i).get());
			while (nb_essais < exemples.size()){
				initInputs(&p, exemples, nb_essais);
				calcOutputs(&p);
				int d = abs(exemples.at(nb_essais).out - findMax(&p));
				/*if (d != 0){
					for (int e = 0; e < NB_ENTREES; e++){
						change(p.neurones.at(e).get());
					}
				}*/
				somme += d;
				nb_essais++;
			}
			population.at(i)->fitness = somme;
			
		}
		if (population.at(findBestIndiv(&population)).get()->fitness < best.fitness){
			best = *population.at(findBestIndiv(&population)).get();
			cout << best.fitness << endl;
		}
		trie(&population);
		reinjection(&population,best);
		newPop(&population);
		for (int i = 0; i < population.size(); i++){
			mutation(&population.at(i));
		}
		
		//selectParents+reinj

		
		//_sleep(100);
		generation++;
	}
	int r, g, b;
	char f = ' ';
	affInfiv(best);
	appliquerIndivAPerceptron(&p, best);
	while (f != 'o')
	{
		cout << "RED : ";
		cin >> r;
		cout << endl << "GREEN : ";
		cin >> g;
		cout << endl << "BLUE : ";
		cin >> b;
		cout << endl;
		setInputs(&p, r, g, b);
		calcOutputs(&p);
		cout << findMax(&p) << endl;
		cout << "FINI ? o pour terminer : ";
		cin >> f;
		cout << endl;
	}
	
	system("PAUSE");
	fich.close();
	return 0;
}

