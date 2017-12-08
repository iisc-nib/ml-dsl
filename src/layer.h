#ifndef _LAYER_H_
#define _LAYER_H_

#include <vector>

class Neuron;


class Layer
{
public:
	Layer(const NeuronList& neurons)
		m_neurons(neurons)
	{ }

	const NeuronList& GetNeurons() { return m_neurons; }

	void AddNeuron(Neuron* neuron) { m_neurons.push_back(neuron); }
	void AddNeurons(const NeuronList& neurons) { m_neurons.insert(m_neurons.end(), neurons.begin(), neurons.end()); }
	void AddNeurons(const Neuron* pNeurons, size_t n) { m_neurons.insert(m_neurons.end(), pNeurons, pNeurons + n); }
private:
	NeuronList m_neurons;
};

#endif // _LAYER_H_  
