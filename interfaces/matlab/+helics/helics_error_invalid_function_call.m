function v = helics_error_invalid_function_call()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 59);
  end
  v = vInitialized;
end
