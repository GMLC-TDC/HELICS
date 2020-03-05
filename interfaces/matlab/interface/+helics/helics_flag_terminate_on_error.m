function v = helics_flag_terminate_on_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 42);
  end
  v = vInitialized;
end
