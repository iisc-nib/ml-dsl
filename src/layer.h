#ifndef _LAYER_H_
#define _LAYER_H_

#include <vector>
#include <algorithm>

class Neuron;
class Network;
class NetworkVisitor;

// TODO should we have subclasses of Layer (Input, hidden, output) so we can construct the right type of neurons automatically?
class Layer
{
    friend class Network;

	Layer()
	{ }
	const NeuronList& GetNeurons() { return m_neurons; }
public:
    Neuron& GetNeuron(int32_t index) { return *m_neurons[index]; }
    Neuron& operator[](int32_t index) { return *m_neurons[index]; }
    int32_t GetNumberOfNeurons() { return static_cast<int32_t>(m_neurons.size()); }

	Neuron& AddNeuron(int32_t& neuronID)
    {
        Neuron* newNeuron = new Neuron(*this);
        neuronID = m_neurons.size();
        m_neurons.push_back(newNeuron);
        return *newNeuron;
    }

    InputNeuron& AddInputNeuron(int32_t& neuronID)
    {
        neuronID = m_neurons.size();
        InputNeuron* newNeuron = new InputNeuron(*this, neuronID);
        m_neurons.push_back(newNeuron);
        return *newNeuron;
    }

    OutputNeuron& AddOutputNeuron(int32_t& neuronID)
    {
        neuronID = m_neurons.size();
        OutputNeuron* newNeuron = new OutputNeuron(*this, neuronID);
        m_neurons.push_back(newNeuron);
        return *newNeuron;
    }

    int32_t GetNeuronID(Neuron& neuron) { return std::find(m_neurons.begin(), m_neurons.end(), &neuron) - m_neurons.begin(); }

    virtual void AcceptVisitor(NetworkVisitor& visitor) { visitor.Visit(*this); }
    // void AddNeuron(Neuron* neuron) { m_neurons.push_back(neuron); }
	// void AddNeurons(NeuronList& neurons) { m_neurons.insert(m_neurons.end(), neurons.begin(), neurons.end()); }
	// void AddNeurons(Neuron** pNeurons, size_t n) { m_neurons.insert(m_neurons.end(), pNeurons, pNeurons + n); }
private:
	NeuronList m_neurons;
};

#endif // _LAYER_H_  
