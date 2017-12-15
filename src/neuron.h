#ifndef _NEURON_H_
#define _NEURON_H_

#include <vector>
#include <list>
#include <cstdint>
#include "networkvisitor.h"

class Layer;
class ValueType;
class Neuron;
class Value;
class NeuronProperty;

typedef std::vector<Neuron*> NeuronList;

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
    friend void ConnectNeurons(Neuron& src, Neuron& sink);
public:

	// int32_t AddScalarDoubleNeuronProperty(double val);
	// int32_t AddVectorDoubleNeuronProperty(std::vector<double>& val);

    NeuronProperty& GetNeuronProperty(int32_t propID) { return *m_properties[propID]; }

    void SetForwardPropagationValue(Value& value) { m_forwardValue = &value; }
    Value& GetForwardPropagationValue() { return *m_forwardValue; }

    NeuronList& GetSources() { return m_sources; }
    NeuronList& GetSinks() { return m_sinks; }

    int32_t GetNumInputs() { return m_sources.size(); }

    Layer& GetLayer() { return m_layer; }

    virtual void CheckTypes();

    virtual void AcceptVisitor(NetworkVisitor& visitor) { visitor.Visit(*this); }

    virtual ~Neuron() { }

protected:
	// The layer that this neuron belongs to
	Layer &m_layer;

	// Represents the expression used to compute the output value
    // of a neuron
    Value* m_forwardValue;

	NeuronList m_sources;
	NeuronList m_sinks;

	std::vector<NeuronProperty*> m_properties;

	Neuron(Layer &layer) 
		:m_layer(layer), m_forwardValue(nullptr)
   	{ }

	void AddSource(Neuron& src) { m_sources.push_back(&src); }
	void AddSink(Neuron& sink) { m_sinks.push_back(&sink); }
};

// TODO Not all functions that are on neuron make sense for input and output neurons. We need to refactor
class InputNeuron : public Neuron
{
    friend class Layer;
    friend void ConnectNeurons(Neuron& src, Neuron& sink);
public:
    int32_t GetIndex() { return m_index; }
    virtual void CheckTypes() { } // Input neurons will always have a real scalar output
    virtual void AcceptVisitor(NetworkVisitor& visitor) { visitor.Visit(*this); }
protected:
    int32_t m_index;
    InputNeuron(Layer& layer, int32_t index)
        :Neuron(layer), m_index(index)
    { }
};

class OutputNeuron : public Neuron
{
    friend class Layer;
    friend void ConnectNeurons(Neuron& src, Neuron& sink);
public:
    int32_t GetIndex() { return m_index; }
    virtual void AcceptVisitor(NetworkVisitor& visitor) { visitor.Visit(*this); }
protected:
    int32_t m_index;
    OutputNeuron(Layer& layer, int32_t index)
        :Neuron(layer), m_index(index)
    { }
};

void ConnectNeurons(Neuron& src, Neuron& sink);

#endif // _NEURON_H_

