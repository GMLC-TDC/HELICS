function v = helics_flag_forward_compute()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 36);
  end
  v = vInitialized;
end
