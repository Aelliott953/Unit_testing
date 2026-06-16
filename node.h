#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QString>
#include <functional>
#include <string>
#include <vector>
#include <sstream>

struct Data {
    std::string data_type;
    std::vector<double> values;
};

Q_DECLARE_METATYPE(Data)

class Node : public QObject
{
    Q_OBJECT

public:
    explicit Node(QObject *parent, const QString &name,
                  std::function<Data(const Data&)> action);

    std::string to_string() const;

public slots:
    void set_value(const Data &data);

signals:
    void value_changed(const Data &data);

private:
    Data value;
    std::function<Data(const Data&)> action;
};

#endif // NODE_H
