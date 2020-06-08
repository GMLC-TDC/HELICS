function v = helics_state_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 121);
  end
  v = vInitialized;
end
