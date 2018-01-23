#include <stdexcept>
#include "valuetype.h"
#include "value.h"
#include "neuronproperty.h"
#include "neuron.h"
#include "layer.h"
#include <limits>

/*
int32_t Neuron::AddScalarDoubleNeuronProperty(double val)
{
	int32_t propID = m_properties.size();
	m_properties.push_back(new ScalarDoubleNeuronProperty(val));
	return propID;
}

int32_t Neuron::AddVectorDoubleNeuronProperty(std::vector<double>& val)
{
	int32_t propID = m_properties.size();
	m_properties.push_back(new VectorDoubleNeuronProperty(val));
	return propID;
}
*/

int32_t Neuron::GetNeuronID()
{
    return m_layer.GetNeuronID(*this);
}

void Neuron::CheckTypes()
{
    if (m_forwardValue == nullptr)
        return;
    ValueType& outputType = m_forwardValue->GetType();
    ScalarType *outputScalarType = dynamic_cast<ScalarType*>(&outputType);
    if (!outputScalarType)
        throw std::runtime_error("Neuron output must be scalar");
}

void ConnectNeurons(Neuron& src, Neuron& sink)
{
    if (dynamic_cast<OutputNeuron*>(&src) != nullptr)
        throw std::runtime_error("An output neuron cannot be a source");

    if (dynamic_cast<InputNeuron*>(&sink) != nullptr)
        throw std::runtime_error("An input neuron cannot be a sink");

    src.AddSink(sink);
    sink.AddSource(src);
}

bool AreNeuronsMergeable(Neuron& n1, Neuron& n2)
{
    bool mergeable = AreValuesStructurallyIdentical(n1.GetForwardPropagationValue(), n2.GetForwardPropagationValue());
    if (!mergeable) return false;

    NeuronList& inputList1 = n1.GetSources();
    NeuronList& inputList2 = n2.GetSources();
    
    // Since we already checked that the neurons have structurally identically forward propagation values, we expect the
    // number of inputs to be the same.
    if (inputList1.size() != inputList2.size())
        throw std::runtime_error("Expected input list lengths to be equal!");
    
    // The difference between the index of every neuron in each input list and the index of the first neuron 
    // in that list must be pairwise equal for all input neurons for the two argument neurons to be mergeable.
    // This is essentially saying that the input neurons have the same "pattern" relative to the first input.
    // Three neurons are mergeable if they are pairwise mergeable (transitivity holds).
    for (NeuronList::iterator iter1=inputList1.begin(), iter2=inputList2.begin() ; iter1!=inputList1.end() ; ++iter1, ++iter2)
    {
        int32_t difference1 = (*iter1)->GetNeuronID() - (*inputList1.begin())->GetNeuronID();
        int32_t difference2 = (*iter2)->GetNeuronID() - (*inputList2.begin())->GetNeuronID();
        if (difference1 != difference2)
            return false;
    }
    
    return true;
}
