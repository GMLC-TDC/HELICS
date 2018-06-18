function v = helics_invalid_function_call()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176311);
  end
  v = vInitialized;
end
