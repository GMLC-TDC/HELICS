function v = helics_ok()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 51);
  end
  v = vInitialized;
end
