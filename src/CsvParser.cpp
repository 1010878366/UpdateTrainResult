#include "CsvParser.h"

QVector<DefectRecord> CsvParser::ParseCsv(QString strCsvPath)
{
    QVector<DefectRecord> vecRecords;
    QFile file(strCsvPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return vecRecords;
    }

    while (!file.atEnd())
    {
        QString strLine = file.readLine().trimmed(); //一行数据并清除首尾空白，避免空行或多余空格对后续解析造成干扰
        if(strLine.isEmpty())
            continue;   //跳过空行
        QStringList parts = strLine.split(',');

        //解析每列数据
        DefectRecord rec;
        rec.nFileIndex = parts.size() > 0 && !parts[0].isEmpty() ? parts[0].toInt() : 0;
        rec.nDefectIndex = parts.size() > 1 && !parts[1].isEmpty() ? parts[1].toInt() : 0;
        rec.nDefectLevel = parts.size() > 2 && !parts[2].isEmpty() ? parts[2].toInt() : 0;
        rec.fCentreX = parts.size() > 3 && !parts[3].isEmpty() ? parts[3].toFloat() : -9.9999f;
        rec.fCentreY = parts.size() > 4 && !parts[4].isEmpty() ? parts[4].toFloat() : -9.9999f;
        rec.fLength  = parts.size() > 5 && !parts[5].isEmpty() ? parts[5].toFloat() : -9.9999f;
        rec.fHeight  = parts.size() > 6 && !parts[6].isEmpty() ? parts[6].toFloat() : -9.9999f;

        vecRecords.append(rec);
    }
    file.close();
    return vecRecords;
}
