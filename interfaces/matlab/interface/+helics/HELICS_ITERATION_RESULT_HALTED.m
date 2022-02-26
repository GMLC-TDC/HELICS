function v = HELICS_ITERATION_RESULT_HALTED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 139);
  end
  v = vInitialized;
end
