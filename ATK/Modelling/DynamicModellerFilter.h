/**
 * \file DynamicModellerFilter.h
 */

#ifndef ATK_MODELLING_DYNAMICMODELLERFILTER_H
#define ATK_MODELLING_DYNAMICMODELLERFILTER_H

#include <tuple>
#include <unordered_set>
#include <vector>

#include <gsl/gsl>

#include <Eigen/Eigen>

#include "config.h"
#include "ModellerFilter.h"

namespace ATK
{
  template<typename DataType_>
  class Component;
  
  /// The main DynamicModellerFilter
  template<typename DataType_>
  class ATK_MODELLING_EXPORT DynamicModellerFilter: public ModellerFilter<DataType_>
  {
  public:
    using Parent = TypedBaseFilter<DataType_>;
    using DataType = DataType_;

    using Parent::input_sampling_rate;
    using Parent::output_sampling_rate;
    using Parent::nb_input_ports;
    using Parent::converted_inputs;
    using Parent::nb_output_ports;
    using Parent::outputs;

  private:
    gsl::index nb_dynamic_pins;
    gsl::index nb_static_pins;
    gsl::index nb_input_pins;
    
    /// vector of dynamic pins, each pin has a list of components connected with it, and the index of the pin for the component
    std::vector<std::vector<std::tuple<Component<DataType>*, gsl::index>>> dynamic_pins;
    /// vector of dynamic pins, indicating if the equation is overriden by a component
    std::vector<std::tuple<Component<DataType>*, gsl::index>> dynamic_pins_equation;

    mutable Eigen::Matrix<DataType, Eigen::Dynamic, 1> dynamic_state;
    Eigen::Matrix<DataType, Eigen::Dynamic, 1> static_state;
    mutable Eigen::Matrix<DataType, Eigen::Dynamic, 1> input_state;

    std::unordered_set<std::unique_ptr<Component<DataType>>> components;
    
    bool initialized = false;
    
    const Eigen::Matrix<DataType, Eigen::Dynamic, 1>& get_states(PinType type) const;

    std::vector<std::string> dynamic_pins_names;
    std::vector<std::string> static_pins_names;

  public:
    /**
     * The main ModellerFilter constructor
     * @param nb_dynamic_pins is the number of dymanic pins (that have a voltage that may vary with time)
     * @param nb_static_pins is the number of static pins (that have a fixed voltage)
     * @param nb_input_pins is the number of input pins (that will have varying voltage with time)
     */
    DynamicModellerFilter(gsl::index nb_dynamic_pins, gsl::index nb_static_pins, gsl::index nb_input_pins);
    
    /// Explicit destructor to avoid more than a forward declaration of Component
    ~DynamicModellerFilter();
    
    using Pin = std::tuple<PinType, gsl::index>;
    
    /**
     * Adds a new component to the model
     * @param component is the new component to add
     * @param pins is a vector with the pins that will be used by the component
     */
    void add_component(std::unique_ptr<Component<DataType>> component, std::vector<Pin> pins);
    
    /**
     * Called during model update to remove a Kirchhoff equation and replace it with a component specific one
     * @param eq is the equation number to replace
     * @param custom_equation is the custom equation reference that we will use
     */
    void set_custom_equation(gsl::index eq, std::tuple<Component<DataType>*, gsl::index> custom_equation);
    
    /**
     * Gets a voltage from one of the states
     * @param pin is the pin to get the voltage for
     */
    DataType retrieve_voltage(const Pin& pin) const;
    
    /**
     * Sets the current static state
     */
    void set_static_state(Eigen::Matrix<DataType, Eigen::Dynamic, 1> static_state);
    
    void set_dynamic_pin_names(std::vector<std::string> dynamic_pins_names)
    {
      this->dynamic_pins_names = std::move(dynamic_pins_names);
    }
    
    void set_static_pin_names(std::vector<std::string> static_pins_names)
    {
      this->static_pins_names = std::move(static_pins_names);
    }

    Eigen::Matrix<DataType, Eigen::Dynamic, 1> get_static_state() const override
    {
      return static_state;
    }
    
    const Eigen::Matrix<DataType, Eigen::Dynamic, 1>& get_dynamic_state() const
    {
      return dynamic_state;
    }
    
    const Eigen::Matrix<DataType, Eigen::Dynamic, 1>& get_input_state() const
    {
      return input_state;
    }

    /// Returns the number of dynamic pins
    gsl::index get_nb_dynamic_pins() const override
    {
      return nb_dynamic_pins;
    }

    /// Returns the number of static pins
    gsl::index get_nb_static_pins() const override
    {
      return nb_static_pins;
    }

    /// Returns the number of input pins
    gsl::index get_nb_input_pins() const override
    {
      return nb_input_pins;
    }

    /// Returns the number of components
    gsl::index get_nb_components() const override
    {
      return components.size();
    }

    /// Returns the name of a dynamic pin, usefull to set output
    std::string get_dynamic_pin_name(gsl::index identifier) const override
    {
      return dynamic_pins_names[identifier];
    }
    
    /// Returns the name of a static pin, usefull to set input
    std::string get_static_pin_name(gsl::index identifier) const override
    {
      return static_pins_names[identifier];
    }
    
    /// Get number of parameters
    gsl::index get_number_parameters() const override;
    
    /// Get the name of a parameter
    std::string get_parameter_name(gsl::index identifier) const override;
    
    /// Get the value of a parameter
    DataType_ get_parameter(gsl::index identifier) const override;
    
    /// Set the value of a parameter
    void set_parameter(gsl::index identifier, DataType_ value) override;

    /**
     * Sets up the internal state of the ModellerFilter
     */
    void init();

    /**
     * Setups internals
     */
    void setup() override;

    /**
     * Computes a new state based on a new set of inputs
     */
    void process_impl(gsl::index size) const override;
    
  private:
    /**
     * Solve the state of the ModellerFilter
     * @param steady_state indicates if a steady state is requested
     */
    void solve(bool steady_state) const;

    /**
     * One iteration for the solver
     * @param steady_state indicates if a steady state is requested
     */
    bool iterate(bool steady_state) const;
    
    /**
     * Retrieve all currents for a given pin and the corresponding jacobian
     * @param i is the pin to update
     * @param eqs is the full equation state, only one element will be updated
     * @params jacobian is the full jacobian, but only one line will be updated
     * @param steady_state indicates if a steady state is requested
     */
    void compute_current(gsl::index i, Eigen::Matrix<DataType, Eigen::Dynamic, 1>& eqs, Eigen::Matrix<DataType, Eigen::Dynamic, Eigen::Dynamic>& jacobian, bool steady_state) const;
  };
}

#endif
