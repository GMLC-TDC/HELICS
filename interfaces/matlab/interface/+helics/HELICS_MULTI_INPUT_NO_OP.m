function v = HELICS_MULTI_INPUT_NO_OP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 98);
  end
  v = vInitialized;
end
