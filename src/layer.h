#ifndef _LAYER_H_
#define _LAYER_H_

#include <vector>
#include <algorithm>

class Neuron;
class Network;
class NetworkVisitor;

class Ensemble
{
    NeuronList m_neurons;
public:
    Ensemble() { }
    ~Ensemble() { }
    void AddNeuron(Neuron& neuron);
    NeuronList& GetNeurons() { return m_neurons; }
    int32_t GetNumberOfNeurons() { return static_cast<int32_t>(m_neurons.size()); }
};

typedef std::vector<Ensemble*> Ensembles; // Clean code!!!

// TODO should we have subclasses of Layer (Input, hidden, output) so we can construct the right type of neurons automatically?
class Layer
{
    friend class Network;

	Layer()
	{ }
	const NeuronList& GetNeurons() { return m_neurons; }
public:

    ~Layer()
    {
        for(NeuronList::iterator iter = m_neurons.begin() ; iter != m_neurons.end(); ++iter)
            delete *iter;
        for(Ensembles::iterator iter = m_ensembles.begin() ; iter != m_ensembles.end() ; ++iter)
            delete *iter;
    }
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
    
    Ensemble& CreateNewEnsemble()
    {
        Ensemble *ensemble = new Ensemble;
        m_ensembles.push_back(ensemble);
        return *ensemble;
    }

    Ensembles& GetEnsembles() { return m_ensembles; }

private:
	NeuronList m_neurons;
    Ensembles m_ensembles;
};

#endif // _LAYER_H_  
