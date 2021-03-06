/**
 * \ file Capacitor.cpp
 */

#include <array>
#include <iostream>

#include <ATK/config.h>

#include <ATK/Core/InPointerFilter.h>
#include <ATK/Core/OutPointerFilter.h>

#include <ATK/Modelling/DynamicModellerFilter.h>
#include <ATK/Modelling/Capacitor.h>
#include <ATK/Modelling/Resistor.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

static constexpr size_t PROCESSSIZE = 100;

static constexpr double R = 1000;
static constexpr double C = 1e-3;
static constexpr double rate = 48e3;
static constexpr double dt = 1/rate;

BOOST_AUTO_TEST_CASE( Capacitor_RC )
{
  std::array<double, PROCESSSIZE> data;
  for(gsl::index i = 0; i < PROCESSSIZE; ++i)
  {
    data[i] = 1;
  }
  
  ATK::InPointerFilter<double> generator(data.data(), 1, PROCESSSIZE, false);
  generator.set_output_sampling_rate(48000);
  
  ATK::DynamicModellerFilter<double> model(1, 1, 1);
  model.add_component(std::make_unique<ATK::Resistor<double>>(R), {{std::make_tuple(ATK::PinType::Input, 0), std::make_tuple(ATK::PinType::Dynamic, 0)}});
  model.add_component(std::make_unique<ATK::Capacitor<double>>(C), {{std::make_tuple(ATK::PinType::Static, 0), std::make_tuple(ATK::PinType::Dynamic, 0)}});

  model.set_input_sampling_rate(48000);
  model.set_output_sampling_rate(48000);
  
  model.set_input_port(0, &generator, 0);
  model.setup();
  
  model.process(PROCESSSIZE);
  
  for(gsl::index i = 0; i < PROCESSSIZE; ++i)
  {
    BOOST_CHECK_CLOSE(1 - std::exp(-(i+.5) * dt / (R * C)), model.get_output_array(0)[i], 1);
  }
}
