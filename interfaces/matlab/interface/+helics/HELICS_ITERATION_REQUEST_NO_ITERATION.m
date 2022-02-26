function v = HELICS_ITERATION_REQUEST_NO_ITERATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 134);
  end
  v = vInitialized;
end
