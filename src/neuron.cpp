#include "valuetype.h"
#include "neuronproperty.h"
#include "neuron.h"

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

