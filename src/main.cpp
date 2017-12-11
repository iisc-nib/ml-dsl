#include <iostream>
#include "mldslapi.h"

// TODO Should this be constructing a Constant rather than a property?
void ConstructWeightedNeuronForwardPropFunction(Neuron& neuron, int32_t numInputs, std::vector<double>& weights, double bias)
{
    Value& x = GetInputValue::Create(neuron);
    int32_t propID = neuron.AddVectorDoubleNeuronProperty(weights);
    Value& w = GetProperty::Create(neuron, propID);

    Value& wTimesx = w*x;
    Value& sumOfwTimesx = Reduction::Create(wTimesx, Reduction::Sum);
    Value& b = Constant(bias);
    Value& out = sumOfwTimesx + b;
    
    neuron.SetForwardPropagationValue(out);
}

Network& ConstructSimpleThreeLayerNet(int32_t numNeurons)
{
    Network& net = Network::Create();
    int32_t layer1ID, layer2ID, layer3ID;
    Layer& inputLayer = net.AddLayer(layer1ID);
    Layer& hiddenLayer = net.AddLayer(layer2ID);
    Layer& outputLayer = net.AddLayer(layer3ID);

    for (int32_t i=0 ; i<numNeurons ; ++i)
    {
        int32_t id = 0;
        InputNeuron& neuron = inputLayer.AddInputNeuron(id);
        neuron.SetForwardPropagationValue(GetInputValue::Create(neuron));
    }

    for (int32_t i=0 ; i<numNeurons ; ++i)
    {
        std::vector<double> w(numNeurons);
        for (int j=0; j<numNeurons ; ++j)
            w[j] = (double)rand()/RAND_MAX;
            
        int32_t id = 0;
        Neuron& neuron = hiddenLayer.AddNeuron(id);
        ConstructWeightedNeuronForwardPropFunction(neuron, numNeurons, w, 1);
    }

    for(int32_t i=0; i<numNeurons ; ++i)
    {
        std::vector<double> w(numNeurons);
        for (int j=0; j<numNeurons ; ++j)
            w[j] = (double)rand()/RAND_MAX;
            
        int32_t id = 0;
        Neuron& neuron = outputLayer.AddOutputNeuron(id);
        ConstructWeightedNeuronForwardPropFunction(neuron, numNeurons, w, 1);
    }

    std::map<int32_t, std::vector<int32_t>> connections;
    for (int32_t i=0 ; i<numNeurons ; ++i)
    {
        std::vector<int32_t> srcs(numNeurons, -1);

        for (int j=0; j<numNeurons ; ++j)
            srcs[j] = j;

        connections[i] = srcs;
    }

    net.ConnectLayers(layer1ID, layer2ID, connections);
    net.ConnectLayers(layer2ID, layer3ID, connections);
    net.CheckTypes();

    PrintNetwork(net, std::cout);
}

int main()
{
	ConstructSimpleThreeLayerNet(4);
    return 0;
}
