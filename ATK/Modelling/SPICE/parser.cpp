/**
 * \file parser.cpp
 */

#include <algorithm>
#include <fstream>

#include <boost/algorithm/string/case_conv.hpp>

#include <ATK/Core/Utilities.h>

#include <ATK/Modelling/SPICE/parser.h>

namespace ATK
{
namespace parser
{
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
using x3::lexeme;

namespace
{
  auto tolower = [](auto& ctx){ _val(ctx) += std::tolower(_attr(ctx)); };
}

auto const space_comment = x3::lexeme[ '#' >> *(x3::char_ - x3::eol) >> x3::eol];

const auto componentName = x3::rule<class name, std::string>()
  = (x3::char_("rR"
               "vV"
               )[tolower] >> *(x3::alnum[tolower] | x3::punct));

const auto componentValue = x3::rule<class componentValue, ast::SPICENumber>()
  = x3::double_ >> *(x3::char_ - x3::space); // to lower is done int he transofrmation function

const auto pin = x3::rule<class pin, std::string>()
  = (x3::alpha[tolower] >> *(x3::alnum[tolower] | x3::punct));
  
const auto componentArg = x3::rule<class componentArg, ast::SPICEArg>()
  = componentValue | pin;

const auto componentArguments = x3::rule<class componentArguments, std::vector<ast::SPICEArg>>()
  = componentArg % +(lit(' ') | lit('\n'));

const auto component = x3::rule<class component, ast::Component>()
  = componentName >> +(lit(' ') | lit('\n')) >> componentArguments >> *(lit(' ') | lit('\n'));

const auto entry = x3::rule<class entry, ast::SPICEEntry>()
  = component;

}

namespace
{
  double convertSuffix(const std::string& suffix)
  {
    if(suffix.empty())
    {
      return 1;
    }
    if(suffix.size() > 2)
    {
      std::string cpy(suffix.substr(0, 3));
      boost::algorithm::to_lower(cpy);
      if(cpy == "mil")
      {
        return 2.54e-6;
      }
      if(cpy == "meg")
      {
        return 1e6;
      }
    }
    switch(std::tolower(suffix[0]))
    {
      case 'f': return 1.e-15;
      case 'p': return 1.e-12;
      case 'n': return 1.e-9;
      case 'u': return 1.e-6;
      case 'm': return 1.e-3;
      case 'k': return 1.e3;
      case 'g': return 1.e9;
      case 't': return 1.e12;
      default:
        return 1;
    }
  }
  
  template <typename ReturnT, typename... Lambdas>
  struct lambda_visitor;
  
  template <typename ReturnT, typename L1, typename... Lambdas>
  struct lambda_visitor< ReturnT, L1 , Lambdas...> : public L1, public lambda_visitor<ReturnT, Lambdas...>
  {
    using L1::operator();
    using lambda_visitor< ReturnT , Lambdas...>::operator();
    lambda_visitor(L1 l1, Lambdas... lambdas)
    : L1(l1), lambda_visitor< ReturnT , Lambdas...> (lambdas...)
    {}
  };
  
  template <typename ReturnT, typename L1>
  struct lambda_visitor<ReturnT, L1> : public L1, public boost::static_visitor<ReturnT>
  {
    using L1::operator();
    lambda_visitor(L1 l1)
    :  L1(l1), boost::static_visitor<ReturnT>()
    {}
  };
  
  template <typename ReturnT>
  struct lambda_visitor<ReturnT> : public boost::static_visitor<ReturnT>
  {
    lambda_visitor() : boost::static_visitor<ReturnT>() {}
  };
  
  template <typename ReturnT, typename... Lambdas>
  lambda_visitor<ReturnT, Lambdas...> make_lambda_visitor(Lambdas... lambdas)
  {
    return { lambdas... };
  }
  
  void populateEntry(ast::SPICEAST& currentAST, ast::SPICEEntry entry)
  {
    auto visitor = make_lambda_visitor<void>(
                                             [&](ast::Component& arg) { currentAST.components.insert(std::move(arg)); },
                                             [&](ast::Model& arg) { }
                                             );
    boost::apply_visitor(visitor, std::move(entry));
  }
}
  
double convertComponentValue(const ast::SPICENumber& value)
{
  return value.first * convertSuffix(value.second);
}

double parseComponentValue(const std::string& str)
{
  auto iter = str.begin();
  auto end = str.end();
  ast::SPICENumber value;
  bool r = phrase_parse(iter, end, parser::componentValue, parser::space_comment, value);
  if(!r)
  {
    throw ATK::RuntimeError("Failed to parse value");
  }
  if(iter != end)
  {
    throw ATK::RuntimeError("Failed to parse line, reminder is " + std::string(iter, end));
  }
  return convertComponentValue(value);
}

void parseString(ast::SPICEAST& currentAST, const std::string& str)
{
  auto iter = str.begin();
  auto end = str.end();
  ast::SPICEEntry entry;
  bool r = phrase_parse(iter, end, parser::entry, parser::space_comment, entry);
  if(!r)
  {
    throw ATK::RuntimeError("Failed to parse line");
  }
  if(iter != end)
  {
    throw ATK::RuntimeError("Failed to parse line, reminder is " + std::string(iter, end));
  }
  populateEntry(currentAST, entry);
}
}
