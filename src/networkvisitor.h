#ifndef _NETWORKVISITOR_H_
#define _NETWORKVISITOR_H_

class Network;
class Layer;
class Neuron;
class InputNeuron;
class OutputNeuron;

class NetworkVisitor
{
public:
    virtual void Visit(Network& network) = 0;
    virtual void Visit(Layer& layer) = 0;
    virtual void Visit(Neuron& neuron) = 0;
    virtual void Visit(InputNeuron& inputNeuron) = 0;
    virtual void Visit(OutputNeuron& outputNeuron) = 0;
};

#endif // _NETWORKVISITOR_H_
