function v = helics_state_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 107);
  end
  v = vInitialized;
end
