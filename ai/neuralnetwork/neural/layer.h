#ifndef LAYER_H
#define LAYER_H

#include <QMap>
#include <vector>

#include "ai/neuralnetwork/neural/neuron.h"

class NeuralNetwork;
class Layer;

using spLayer = std::shared_ptr<Layer>;
//Layer of the network
class Layer final : public FileSerializable
{
public:
    static const char* const LAYER_PARAMETER_SIZE;
    static const char* const LAYER_PARAMETER_TYPE;
    static const char* const LAYER_PARAMETER_ACTIVATION;
    enum class LayerType
    {
        STANDARD = 0, //Standard layer : fully connected perceptrons
        OUTPUT, // Output : No bias neuron
        INPUT, // Input: Standard input (output of neurons is outputRaw() )
        SOFTMAX //K-Class Classification Layer

    };
    Layer(qint32 id_layer, NeuralNetwork* net, QMap<QString, double> & parameters);
    ~Layer() = default;
    /**
     * @brief extend
     * @param count
     * @param randomize
     */
    void extend(quint32 count, bool randomize);

    qint32 getId() const;

    void initLayer();

    void clean();

    void trigger();

    void connectComplete(Layer* next);

    std::vector<double> output();

    void randomizeAllWeights(double abs_value);
    void mutateAllWeights(double mutationChance, double maxWeight, double mutationRate = 0.1f);
    void setAccumulated(qint32 neuron, double value);
    QString toString();

    const QMap<QString, double>& getParameters() const;

    LayerType getType() const;

    Neuron::ActivationFunction getActivation() const;

    NeuralNetwork* getNet() const { return m_net; }

    Layer *getPreviousLayer() const;
    void setPreviousLayer(Layer *previousLayer);
    /**
     * @brief getNeuron
     * @param id
     * @return
     */
    Neuron* getNeuron(qint32 id);
    /**
     * @brief serialize stores the object
     * @param pStream
     */
    virtual void serializeObject(QDataStream& pStream) const override;
    /**
     * @brief deserialize restores the object
     * @param pStream
     */
    virtual void deserializeObject(QDataStream& pStream) override;
    /**
     * @brief getVersion version of the file
     * @return
     */
    virtual qint32 getVersion() const override
    {
        return 1;
    }
private:
    NeuralNetwork* m_net{nullptr};
    Layer* m_previousLayer{nullptr};
    qint32 m_id_layer;
    std::vector<spNeuron> m_neurons;
    LayerType m_type;
    Neuron::ActivationFunction m_activation;
    QMap<QString, double> m_parameters;
};

#endif // LAYER_H
