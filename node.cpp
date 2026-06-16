#include "node.h"
#include <iostream>

Node::Node(QObject *parent, const QString &name,
           std::function<Data(const Data&)> action)
    : QObject(parent), action(action)
{
    setObjectName(name);
}

std::string Node::to_string() const
{
    std::ostringstream oss;
    oss << objectName().toStdString() << " [" << value.data_type << "]: { ";
    for (const auto &v : value.values) {
        oss << v << " ";
    }
    oss << "}";
    return oss.str();
}

void Node::set_value(const Data &data)
{
    value = data;
    std::cout << "IN  " << to_string() << "\n";

    Data result = action(data);
    value = result;
    std::cout << "OUT " << to_string() << "\n";

    emit value_changed(value);
}
