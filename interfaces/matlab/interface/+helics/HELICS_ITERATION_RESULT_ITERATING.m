function v = HELICS_ITERATION_RESULT_ITERATING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 140);
  end
  v = vInitialized;
end
