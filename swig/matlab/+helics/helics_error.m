function v = helics_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783845);
  end
  v = vInitialized;
end
