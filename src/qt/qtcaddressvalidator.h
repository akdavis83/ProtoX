// Copyright (c) 2011-2020 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QTC_QT_QTCADDRESSVALIDATOR_H
#define QTC_QT_QTCADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class Quantum CoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit Quantum CoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** Quantum Coin address widget validator, checks for a valid qtc address.
 */
class Quantum CoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit Quantum CoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // QTC_QT_QTCADDRESSVALIDATOR_H
