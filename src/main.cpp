#include <iostream>
#include <cassert>
#include "mldslapi.h"

void ConstructWeightedNeuronForwardPropFunction(Neuron& neuron, std::vector<double>& weights, double bias)
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

void AddWeightedNeuronsToLayer(Layer& layer, int32_t numNeurons)
{
    for (int32_t i=0 ; i<numNeurons ; ++i)
    {
        std::vector<double> w(numNeurons);
        for (int j=0; j<numNeurons ; ++j)
            w[j] = (double)rand()/RAND_MAX;
            
        int32_t id = 0;
        Neuron& neuron = layer.AddNeuron(id);
        ConstructWeightedNeuronForwardPropFunction(neuron, w, 1);
    }
}

void ConstructSimpleThreeLayerNet(int32_t numNeurons)
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

    AddWeightedNeuronsToLayer(hiddenLayer, numNeurons);

    for(int32_t i=0; i<numNeurons ; ++i)
    {
        std::vector<double> w(numNeurons);
        for (int j=0; j<numNeurons ; ++j)
            w[j] = (double)rand()/RAND_MAX;
            
        int32_t id = 0;
        Neuron& neuron = outputLayer.AddOutputNeuron(id);
        ConstructWeightedNeuronForwardPropFunction(neuron, w, 1);
    }

    /*
    std::map<int32_t, std::vector<int32_t>> connections;
    for (int32_t i=0 ; i<numNeurons ; ++i)
    {
        std::vector<int32_t> srcs(numNeurons, -1);

        for (int j=0; j<numNeurons ; ++j)
            srcs[j] = j;

        connections[i] = srcs;
    }
    */

    net.FullyConnectLayers(layer1ID, layer2ID);
    net.FullyConnectLayers(layer2ID, layer3ID);
    net.CheckTypes();
    
    CollectMergeableNeuronsIntoEnsembles(net);
    PrintNetwork(net, std::cout);
    // std::cout << AreNeuronsMergeable(hiddenLayer.GetNeuron(0), hiddenLayer.GetNeuron(1)) << std::endl;
    Network::Destroy(net);
}

void TestConvolutionalNet(int32_t numInputNeurons, int32_t blockSize)
{
    Network& net = Network::Create();
    int32_t layer1ID, layer2ID, layer3ID;
    Layer& inputLayer = net.AddLayer(layer1ID);
    Layer& hiddenLayer = net.AddLayer(layer2ID);
    Layer& outputLayer = net.AddLayer(layer3ID);

    for (int32_t i=0 ; i<numInputNeurons ; ++i)
    {
        int32_t id = 0;
        InputNeuron& neuron = inputLayer.AddInputNeuron(id);
        neuron.SetForwardPropagationValue(GetInputValue::Create(neuron));
    }

    int32_t numNeuronsInHiddenLayer = numInputNeurons - blockSize + 1;
    AddWeightedNeuronsToLayer(hiddenLayer, numNeuronsInHiddenLayer);

    int32_t numOutputNeurons = numInputNeurons;
    for(int32_t i=0; i<numOutputNeurons ; ++i)
    {
        std::vector<double> w(numNeuronsInHiddenLayer);
        for (int j=0; j<numNeuronsInHiddenLayer ; ++j)
            w[j] = (double)rand()/RAND_MAX;
            
        int32_t id = 0;
        Neuron& neuron = outputLayer.AddOutputNeuron(id);
        ConstructWeightedNeuronForwardPropFunction(neuron, w, 1);
    }

    net.ConnectConvolutionalLayer(layer1ID, layer2ID, 0, blockSize);
    net.FullyConnectLayers(layer2ID, layer3ID);

    // PrintNetwork(net, std::cout);
    std::cout << AreNeuronsMergeable(hiddenLayer.GetNeuron(0), outputLayer.GetNeuron(1)) << std::endl;
    Network::Destroy(net);
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

        assert(AreValuesStructurallyIdentical(z1, z2));
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
        assert(!AreValuesStructurallyIdentical(z1, z2));
    }
}

void TestIRValuesAndStatements()
{
    {
        auto start = Constant(0);
        auto end = Constant(10);
        auto forLoop = ForLoop::Create(start, end);
        auto x = Variable::Create("x", *(new IntegerType)); // int x
        auto arr = Variable::Create("arr", *(new VectorType(*(new IntegerType), 10))); // int arr[10]
        auto arrRef = IndexedValue(arr, forLoop.GetIndexVariable());
        auto assignmentRHS = x + arrRef;
        auto assignmentStm = Assignment::Create(x, assignmentRHS);
        forLoop.AddStatement(assignmentStm);
        Print(forLoop, std::cout);
    }
}

int main()
{
	ConstructSimpleThreeLayerNet(4);
    // TestConvolutionalNet(5, 3);
    // TestValueComparison();
    // TestIRValuesAndStatements();
    return 0;
}
