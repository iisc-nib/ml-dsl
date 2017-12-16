#include <iostream>
#include "mldslapi.h"

// TODO Should this be constructing a Constant rather than a property?
void ConstructWeightedNeuronForwardPropFunction(Neuron& neuron, int32_t numInputs, std::vector<double>& weights, double bias)
{
    Value& x = GetInputValue::Create(neuron);
    Value& w = Constant(weights);

    Value& wTimesx = w*x;
    Value& sumOfwTimesx = Reduction::Create(wTimesx, Reduction::Sum);
    Value& b = Constant(bias);
    Value& linearCombination = sumOfwTimesx + b;
    Value& out = ActivationFunction::Create(linearCombination, "sigmoid");
    
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
    std::cout << AreValuesStructurallyIdentical(net[1][0].GetForwardPropagationValue(), net[1][1].GetForwardPropagationValue()) << std::endl;
}

void TestValueComparison()
{
    {
        auto x1 = Constant(2);
        auto y1 = Constant(3);
        auto z1 = x1 + y1;

        auto x2 = Constant(3);
        auto y2 = Constant(4);
        auto z2 = x2 + y2;

        std::cout << AreValuesStructurallyIdentical(z1, z2) << std::endl;
    }
    {
        std::vector<double> x1Val(5, 0);
        std::vector<double> w1Val(5, 0);
        auto x1 = Constant(x1Val);
        auto w1 = Constant(w1Val);
        auto z1 = w1 * x1;

        std::vector<double> x2Val(5, 0);
        std::vector<double> w2Val(5, 0);
        auto x2 = Constant(x2Val);
        auto w2 = Constant(w2Val);
        auto z2 = w2 + x2;
        std::cout << AreValuesStructurallyIdentical(z1, z2) << std::endl;
    }
}

int main()
{
	ConstructSimpleThreeLayerNet(4);
    TestValueComparison();
    return 0;
}
