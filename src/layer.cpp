#include "neuron.h"
#include "layer.h"

void Ensemble::AddNeuron(Neuron& neuron)
{ 
    m_neurons.push_back(&neuron);
    neuron.SetEnsemble(this);
}

