#ifndef _NEURON_H_
#define _NEURON_H_

#include <vector>
#include <cstdint>

class Layer;
class ValueType;
class Neuron;

typedef std::vector<Neuron*> NeuronList;

class NeuronProperty;


// Neuron needs to have a forward propagation function specified in our IR
// Specialized neuron types like weighted neuron will automatically be 
// decomposed into the IR by the compiler. Users are responsible for providing
// the forward propagation function for custom neurons.

// Neurons also need to provide a way for users to add properties on them. For
// example, the weights of the weighted neuron are a property on the neuron.
// For the time being, we will allow properties to be vectors or scalars. We 
// need to decide how users will access these values.

// Predefined IR "methods"
//	- GetScalarPropertyValue(propertyID) -- Get scalar property value
//	- GetVectorElement(propertyID, index) -- Get the "index"th element of a vector property
//	- GetInput(index) -- Get the value of the "index"th input

// The framework will also provide users specialized ways to read a neurons
// inputs and write its output. These will be specialized instructions in our IR.

class Neuron
{
	friend class Layer;
public:
	void AddSource(Neuron& src);
	void AddSink(Neuron& sink);

	int32_t AddScalarDoubleNeuronProperty(double val);
	int32_t AddVectorDoubleNeuronProperty(std::vector<double>& val);

protected:
	
	Layer &m_layer;
	
	NeuronList m_sources;
	NeuronList m_sinks;

	std::vector<NeuronProperty*> m_properties;

	Neuron(Layer &layer) 
		:m_layer(layer)
   	{ }
};

#endif // _NEURON_H_

