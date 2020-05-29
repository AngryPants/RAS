#ifndef RAS_MATH_UTIL_H
#define RAS_MATH_UTIL_H

template <typename T>
T Max(T _value0, T _value1) {
    return (_value0 < _value1) ? _value0 : _value1;
}

template <typename T>
T Min(T _value0, T _value1) {
    return (_value0 < _value1) ? _value0 : _value1;
}

template <typename T>
T Clamp(T _value, T _min, T _max) {
    if (_value < _min) { return _min; }
    if (_value > _max) { return _max; }
    return _value;
}

#endif