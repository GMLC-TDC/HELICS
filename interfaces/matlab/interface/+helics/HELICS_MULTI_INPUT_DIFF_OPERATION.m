function v = HELICS_MULTI_INPUT_DIFF_OPERATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 102);
  end
  v = vInitialized;
end
