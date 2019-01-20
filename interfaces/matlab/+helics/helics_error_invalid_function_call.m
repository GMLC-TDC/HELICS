function v = helics_error_invalid_function_call()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812677);
  end
  v = vInitialized;
end
