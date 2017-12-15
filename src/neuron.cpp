#include <stdexcept>
#include "valuetype.h"
#include "value.h"
#include "neuronproperty.h"
#include "neuron.h"

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
