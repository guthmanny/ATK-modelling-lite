#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from numpy.testing import assert_almost_equal

from ATK.Modelling import *

def TransistorNPN_test():
    model = Modeler(3, 2, 0)

    model.add_component(Resistor(1470), [('S', 0), ('D', 0)])
    model.add_component(Resistor(16670), [('D', 0), ('S', 1)])
    model.add_component(Resistor(1000), [('S', 1), ('D', 1)])
    model.add_component(Resistor(100), [('S', 0), ('D', 2)])
    model.add_component(TransistorNPN(), [('D', 0), ('D', 1), ('D', 2)])
    model.static_state[:] = (0, 5)

    model.dt = 1.e-3
    model.setup()
    
    assert_almost_equal(model.dynamic_state, [4.0510472e-01, 4.9942854e+00, 5.7717722e-04])

def TransistorPNP_test():
    model = Modeler(3, 2, 0)

    model.add_component(Resistor(1470), [('S', 0), ('D', 0)])
    model.add_component(Resistor(16670), [('D', 0), ('S', 1)])
    model.add_component(Resistor(1000), [('S', 1), ('D', 1)])
    model.add_component(Resistor(100), [('S', 0), ('D', 2)])
    model.add_component(TransistorPNP(), [('D', 0), ('D', 1), ('D', 2)])
    model.static_state[:] = (0, -5)

    model.dt = 1.e-3
    model.setup()
    
    assert_almost_equal(model.dynamic_state, [-4.0510472e-01, -4.9942854e+00, -5.7717722e-04])

def MatchedTransistor_test():
    model = Modeler(1, 3, 0)

    model.add_component(Resistor(200e3), [('S', 0), ('D', 0)])
    model.add_component(TransistorNPN(), [('S', 0), ('S', 1), ('D', 0)])
    model.add_component(TransistorPNP(), [('S', 0), ('S', 2), ('D', 0)])
    model.static_state[:] = (1, 2, 0)

    model.dt = 1.e-3
    model.setup()
    
    assert_almost_equal(model.dynamic_state, [1], 4)

