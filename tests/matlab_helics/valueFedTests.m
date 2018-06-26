function tests=valueFedTests

tests=functiontests(localfunctions);
end

function setup(testCase)  % do not change function name
% open a figure, for example

end

function teardown(testCase)  % do not change function name
% close figure, for example
end

function testBasic(testCase)
import matlab.unittest.constraints.IsGreaterThan
ver=helics.helicsGetVersion();
verifyThat(testCase,length(ver),IsGreaterThan(10),'version length is not valid');
end

function testTwo(testCase)
verifyEqual(testCase,2,2);
end

function fedStruct=generateFed(count)
end