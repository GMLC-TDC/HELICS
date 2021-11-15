function v = HELICS_ITERATION_RESULT_HALTED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 134);
  end
  v = vInitialized;
end
