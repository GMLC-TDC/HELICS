function v = HELICS_MULTI_INPUT_AND_OPERATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 101);
  end
  v = vInitialized;
end
