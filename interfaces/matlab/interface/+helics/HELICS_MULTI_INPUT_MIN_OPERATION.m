function v = HELICS_MULTI_INPUT_MIN_OPERATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 104);
  end
  v = vInitialized;
end
