#ifndef _NEURONPROPERTY_H_
#define _NEURONPROPERTY_H_

#include <cstdint>
#include <vector>

class ValueType;

// [TODO] What is the difference between a Neuron property and a constant value?
// [TODO] Do we need to keep a type with each of these properties?
class NeuronProperty
{
protected:
	NeuronProperty() { }
public:
    virtual ~NeuronProperty() { }
};

class ScalarDoubleNeuronProperty : public NeuronProperty
{
	double m_val;
public:
	ScalarDoubleNeuronProperty(double val)
		:m_val(val)
	{ }
	double GetValue() { return m_val; }
};

class VectorDoubleNeuronProperty : public NeuronProperty
{
	std::vector<double> m_val;
public:
	VectorDoubleNeuronProperty(std::vector<double>& val)
		:m_val(val)
	{ }
	std::vector<double>& GetValue() { return m_val; }
	double GetValue(int32_t i) { return m_val[i]; }
};

#endif // _NEURONPROPERTY_H_
