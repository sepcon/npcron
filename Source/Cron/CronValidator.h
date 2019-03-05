#pragma once

#include <string>
#include <regex>

namespace Cron
{

    enum FieldType { cfMin = 0, cfHour, cfMDay, cfMon, cfWDay, cfFieldCount };
    enum class ExpType
	{
		AnyValue,
		SingleValue,
		StepFrom,
		StepWholeRange,
		Range,
        StepFromTo,
		Invalid
	};

    struct ExpInfo
    {
        ExpInfo(std::string expr_, ExpType type_ = ExpType::Invalid, int startValue_ = 0, int endValue_ = 0, int step_ = 1)
            : expr(expr_), type(type_), startValue(startValue_), endValue(endValue_), step(step_){}
        std::string expr;
        ExpType type;
        int startValue;
        int endValue;
        int step;
    };

    using BadSyntaxException = std::string;

    class Validator
	{
	public:
        Validator();
        Validator(const std::string& expression, FieldType field);
        const std::vector<ExpInfo>& subExpressionInfo() const;
        bool isValid() const { return _valid; }
        void changeExpression(const std::string& expression, FieldType field);

	protected:
		bool validate();
		bool isValidFiedValue(int value);
        bool isValidRange(int min, int max, int step = -1);
        bool validateSubExpression(const std::string& subExpr);

        std::vector<ExpInfo> _subExprInfo;
		std::string _expression;
		std::regex _exprRegex;
		std::regex _subExprRegex;
        FieldType _field;
		bool _valid;

		static const std::string constUnitValidRegexStr;
		static const std::string constSubUnitValidRegexStr;
        static const std::string constFieldValidationRegexStr;
        static const std::string constSubFieldValidationRegexStr;
	};

}

