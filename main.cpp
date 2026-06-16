#include <QCoreApplication>
#include <iostream>
#include "node.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<Data>("Data");

    // --- Build node hierarchy ---
    // Generator is the root (no parent)
    Node *generator = new Node(nullptr, "Generator", [](const Data &data) {
        return data;
    });

    // Chain 1: Generator -> Processor_1_1 -> Processor_1_2
    Node *processor_1_1 = new Node(generator, "Processor_1_1", [](const Data &data) {
        Data result = data;
        for (auto &v : result.values)
            v *= 2.0;
        return result;
    });

    Node *processor_1_2 = new Node(processor_1_1, "Processor_1_2", [](const Data &data) {
        Data result = data;
        for (auto &v : result.values)
            v = v * v;
        return result;
    });

    // Chain 2: Generator -> Processor_2_1 -> Processor_2_2
    Node *processor_2_1 = new Node(generator, "Processor_2_1", [](const Data &data) {
        Data result = data;
        for (auto &v : result.values)
            v += 10.0;
        return result;
    });

    Node *processor_2_2 = new Node(processor_2_1, "Processor_2_2", [](const Data &data) {
        Data result = data;
        for (auto &v : result.values)
            v /= 2.0;
        return result;
    });

    // --- Connect signals/slots ---
    QObject::connect(generator,     &Node::value_changed, processor_1_1, &Node::set_value);
    QObject::connect(processor_1_1, &Node::value_changed, processor_1_2, &Node::set_value);
    QObject::connect(generator,     &Node::value_changed, processor_2_1, &Node::set_value);
    QObject::connect(processor_2_1, &Node::value_changed, processor_2_2, &Node::set_value);

    // --- Test ---
    Data input;
    input.data_type = "double";
    input.values    = {1.0, 2.0, 3.0, 4.0, 5.0};

    std::cout << "=== Triggering Generator with input: { 1 2 3 4 5 } ===\n\n";
    generator->set_value(input);
    std::cout << "\n=== Done ===\n";

    delete generator;
    return 0;
}
